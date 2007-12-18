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
//                           avtSiloFileFormat.C                             //
// ************************************************************************* //
#if !defined(DISABLE_EXTENTS_DUE_TO_BAD_ALE3D_FILES)
#define DISABLE_EXTENTS_DUE_TO_BAD_ALE3D_FILES 1
#endif

#include <avtSiloFileFormat.h>

// includes from visit_vtk/full
#ifndef MDSERVER
#include <vtkCSGGrid.h>
#endif

#include <float.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtFacelist.h>
#include <avtGhostData.h>
#include <avtIntervalTree.h>
#include <avtIOInformation.h>
#include <avtMaterial.h>
#include <avtSpecies.h>
#include <avtMixedVariable.h>
#include <avtResampleSelection.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <Utility.h>
#include <Expression.h>
#include <StringHelpers.h>

#include <BadDomainException.h>
#include <BadIndexException.h>
#include <BufferConnection.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <InvalidZoneTypeException.h>
#include <SiloException.h>

#include <avtStructuredDomainBoundaries.h>

#include <visit-config.h>

#include <snprintf.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using std::string;
using std::vector;
using std::map;
using std::set;

static void      ExceptionGenerator(char *);
static char     *GenerateName(const char *, const char *);
static string    PrepareDirName(const char *, const char *);
static void      SplitDirVarName(const char *dirvar, const char *curdir,
                                 string &dir, string &var);

static void      AddDefvars(const char *, avtDatabaseMetaData *);

static int  SiloZoneTypeToVTKZoneType(int);
static void TranslateSiloWedgeToVTKWedge(const int *, vtkIdType [6]);
static void TranslateSiloPyramidToVTKPyramid(const int *, vtkIdType [5]);
static void TranslateSiloTetrahedronToVTKTetrahedron(const int *,
                                                     vtkIdType [4]);
static bool TetsAreInverted(const int *siloTetrahedron,
                            vtkUnstructuredGrid *ugrid);

static int  ComputeNumZonesSkipped(vector<int>& zoneRangesSkipped);

template<class T>
static void RemoveValuesForSkippedZones(vector<int>& zoneRangesSkipped,
                T *inArray, int inArraySize, T *outArray); 

static string GuessCodeNameFromTopLevelVars(DBfile *dbfile);
static void AddAle3drlxstatEnumerationInfo(avtScalarMetaData *smd);

bool avtSiloFileFormat::madeGlobalSiloCalls = false;

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
// ****************************************************************************

avtSiloFileFormat::avtSiloFileFormat(const char *toc_name)
    : avtSTMDFileFormat(&toc_name, 1)
{
    dontForceSingle = 0;

    if (!madeGlobalSiloCalls)
    {
#ifdef PARALLEL
        if (PAR_Rank() == 0)
#endif
            dontForceSingle = getenv("VISIT_SILO_DONT_FORCE_SINGLE") != 0;
#ifdef PARALLEL
        BroadcastInt(dontForceSingle);
#endif

        //
        // Take no chances with precision errors.
        //
        if (dontForceSingle == 0)
            DBForceSingle(1);
    
        //
        // If there is ever a problem with Silo, we want it to throw an
        // exception.
        //
        DBShowErrors(DB_ALL, ExceptionGenerator);
        madeGlobalSiloCalls = true;

        //
        // Turn on silo's checksumming feature. This is harmless if the
        // underlying file DOES NOT contain checksums and will cause only
        // a small performance hit if it does.
        //
#ifdef E_CHECKSUM
        DBSetEnableChecksums(1);
#endif

    }

    dbfiles = new DBfile*[MAX_FILES];
    for (int i = 0 ; i < MAX_FILES ; i++)
    {
        dbfiles[i] = NULL;
    }

    //
    // We sent the toc name in as the only file, so it will end up as index 0.
    //
    tocIndex = 0;

    readGlobalInfo = false;
    connectivityIsTimeVarying = false;
    groupInfo.haveGroups = false;
    hasDisjointElements = false;

#ifdef PARALLEL
    canDoDynamicLoadBalancing = false;
#endif
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
// ****************************************************************************
void
avtSiloFileFormat::ActivateTimestep(void)
{
    OpenFile(tocIndex);
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

    debug4 << "Opening silo file " << filenames[f] << endl;

    //
    // Open the Silo file. Impose priority order on drivers by first
    // trying PDB, then HDF5, then fall-back to UNKNOWN
    //
    if (((dbfiles[f] = DBOpen(filenames[f], DB_PDB, DB_READ)) == NULL) && 
        ((dbfiles[f] = DBOpen(filenames[f], DB_HDF5, DB_READ)) == NULL) && 
        ((dbfiles[f] = DBOpen(filenames[f], DB_UNKNOWN, DB_READ)) == NULL))
    {
        EXCEPTION1(InvalidFilesException, filenames[f]);
    }

    //
    // Lets try to make absolutely sure this is really a Silo file and
    // not just a PDB file that PD_Open succeeded on.
    //
    if (!DBInqVarExists(dbfiles[f], "_silolibinfo")) // newer silo files have this
    {
        //
        // See how many silo objects we have
        //
        DBtoc *toc = DBGetToc(dbfiles[f]);
        int nSiloObjects = toc->nmultimesh + toc->ncsgmesh + toc->nqmesh +
                toc->nucdmesh + toc->nptmesh + toc->nmultivar + toc->ncsgvar +
                toc->nqvar + toc->nucdvar + toc->nptvar + toc->nmat +
                toc->nmultimat + toc->nmatspecies + toc->nmultimatspecies +
                toc->ndir + toc->ndefvars + toc->ncurve;

        //
        // We don't appear to have any silo objects, so we'll fail it
        //
        if (nSiloObjects <= 0)
        {
            char str[1024];
            SNPRINTF(str, sizeof(str), "Although the Silo library succesfully opened \"%s,\"\n" 
                 "the file contains no silo objects. It may be a PDB file.");
            EXCEPTION1(InvalidFilesException, str);
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
    const bool canSkipGlobalInfo = true;
    DBfile *dbfile = OpenFile(tocIndex, canSkipGlobalInfo);
    return ::GetCycle(dbfile);
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
    const bool canSkipGlobalInfo = true;
    DBfile *dbfile = OpenFile(tocIndex, canSkipGlobalInfo);
    return ::GetTime(dbfile);
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
//    I made it use SLASH_STRING so it works better on Windows.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Added bool to skip global info
//
// *****************************************************************************

DBfile *
avtSiloFileFormat::OpenFile(const char *n, bool skipGlobalInfo)
{
    //
    // The directory of this file is all relative to the directory of the 
    // table of contents.  Reflect that here.
    //
    char name[1024];
    char *tocFile = filenames[tocIndex];
    char *thisSlash = tocFile, *lastSlash = tocFile;
    while (thisSlash != NULL)
    {
        lastSlash = thisSlash;
        thisSlash = strstr(lastSlash+1, SLASH_STRING);
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

    for (i = 0 ; i < multimeshes.size() ; i++)
        DBFreeMultimesh(multimeshes[i]);
    multimeshes.clear();
    multimesh_name.clear();

    for (i = 0 ; i < multivars.size() ; i++)
        DBFreeMultivar(multivars[i]);
    multivars.clear();
    multivar_name.clear();

    for (i = 0 ; i < multimats.size() ; i++)
        DBFreeMultimat(multimats[i]);
    multimats.clear();
    multimat_name.clear();

    for (i = 0 ; i < multimatspecies.size() ; i++)
        DBFreeMultimatspecies(multimatspecies[i]);
    multimatspecies.clear();
    multimatspec_name.clear();
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

    //
    // We're just interested in metadata for now, so tell Silo not
    // to read the extra data arrays, except for material names and 
    // numbers and colors.
    //
    DBSetDataReadMask(DBMatMatnames|DBMatMatnos|DBMatMatcolors);

    //
    // Do a recursive search through the subdirectories.
    //
    ReadDir(dbfile, "/", md);
    BroadcastGlobalInfo(md);
    DoRootDirectoryWork(md);

    //
    // Set time/cycle information
    //
    GetTimeVaryingInformation(dbfile, md);

    // To be nice to other functions, tell Silo to turn back on reading all
    // of the data.
    DBSetDataReadMask(DBAll);
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
//  Programmer:  Hank Childs
//  Creation:    October 30, 2001
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
// ****************************************************************************

void
avtSiloFileFormat::ReadDir(DBfile *dbfile, const char *dirname,
                           avtDatabaseMetaData *md)
{
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return;
#endif

    int    i, j, k;
    DBtoc *toc = DBGetToc(dbfile);
    if (toc == NULL)
        EXCEPTION1(InvalidFilesException, filenames[0]);

    //
    // Copy the toc to account for Silo shortcomings.
    //
    int      nmultimesh      = toc->nmultimesh;
    char   **multimesh_names = new char*[nmultimesh];
    for (i = 0 ; i < nmultimesh ; i++)
    {
        multimesh_names[i] = new char[strlen(toc->multimesh_names[i])+1];
        strcpy(multimesh_names[i], toc->multimesh_names[i]);
    }
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    int      ncsgmesh      = toc->ncsgmesh;
    char   **csgmesh_names = new char*[ncsgmesh];
    for (i = 0 ; i < ncsgmesh ; i++)
    {
        csgmesh_names[i] = new char[strlen(toc->csgmesh_names[i])+1];
        strcpy(csgmesh_names[i], toc->csgmesh_names[i]);
    }
#endif
    int      nqmesh      = toc->nqmesh;
    char   **qmesh_names = new char*[nqmesh];
    for (i = 0 ; i < nqmesh ; i++)
    {
        qmesh_names[i] = new char[strlen(toc->qmesh_names[i])+1];
        strcpy(qmesh_names[i], toc->qmesh_names[i]);
    }
    int      nucdmesh      = toc->nucdmesh;
    char   **ucdmesh_names = new char*[nucdmesh];
    for (i = 0 ; i < nucdmesh ; i++)
    {
        ucdmesh_names[i] = new char[strlen(toc->ucdmesh_names[i])+1];
        strcpy(ucdmesh_names[i], toc->ucdmesh_names[i]);
    }
    int      nptmesh      = toc->nptmesh;
    char   **ptmesh_names = new char*[nptmesh];
    for (i = 0 ; i < nptmesh ; i++)
    {
        ptmesh_names[i] = new char[strlen(toc->ptmesh_names[i])+1];
        strcpy(ptmesh_names[i], toc->ptmesh_names[i]);
    }
    int      nmultivar      = toc->nmultivar;
    char   **multivar_names = new char*[nmultivar];
    for (i = 0 ; i < nmultivar ; i++)
    {
        multivar_names[i] = new char[strlen(toc->multivar_names[i])+1];
        strcpy(multivar_names[i], toc->multivar_names[i]);
    }
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    int      ncsgvar      = toc->ncsgvar;
    char   **csgvar_names = new char*[ncsgvar];
    for (i = 0 ; i < ncsgvar ; i++)
    {
        csgvar_names[i] = new char[strlen(toc->csgvar_names[i])+1];
        strcpy(csgvar_names[i], toc->csgvar_names[i]);
    }
#endif
    int      nqvar      = toc->nqvar;
    char   **qvar_names = new char*[nqvar];
    for (i = 0 ; i < nqvar ; i++)
    {
        qvar_names[i] = new char[strlen(toc->qvar_names[i])+1];
        strcpy(qvar_names[i], toc->qvar_names[i]);
    }
    int      nucdvar      = toc->nucdvar;
    char   **ucdvar_names = new char*[nucdvar];
    for (i = 0 ; i < nucdvar ; i++)
    {
        ucdvar_names[i] = new char[strlen(toc->ucdvar_names[i])+1];
        strcpy(ucdvar_names[i], toc->ucdvar_names[i]);
    }
    int      nptvar      = toc->nptvar;
    char   **ptvar_names = new char*[nptvar];
    for (i = 0 ; i < nptvar ; i++)
    {
        ptvar_names[i] = new char[strlen(toc->ptvar_names[i])+1];
        strcpy(ptvar_names[i], toc->ptvar_names[i]);
    }
    int      nmat      = toc->nmat;
    char   **mat_names = new char*[nmat];
    for (i = 0 ; i < nmat ; i++)
    {
        mat_names[i] = new char[strlen(toc->mat_names[i])+1];
        strcpy(mat_names[i], toc->mat_names[i]);
    }
    int      nmultimat      = toc->nmultimat;
    char   **multimat_names = new char*[nmultimat];
    for (i = 0 ; i < nmultimat ; i++)
    {
        multimat_names[i] = new char[strlen(toc->multimat_names[i])+1];
        strcpy(multimat_names[i], toc->multimat_names[i]);
    }
    int      nmatspecies      = toc->nmatspecies;
    char   **matspecies_names = new char*[nmatspecies];
    for (i = 0 ; i < nmatspecies ; i++)
    {
        matspecies_names[i] = new char[strlen(toc->matspecies_names[i])+1];
        strcpy(matspecies_names[i], toc->matspecies_names[i]);
    }
    int      nmultimatspecies      = toc->nmultimatspecies;
    char   **multimatspecies_names = new char*[nmultimatspecies];
    for (i = 0 ; i < nmultimatspecies ; i++)
    {
        multimatspecies_names[i]
                   = new char[strlen(toc->multimatspecies_names[i])+1];
        strcpy(multimatspecies_names[i], toc->multimatspecies_names[i]);
    }
    int      ndir      = toc->ndir;
    char   **dir_names = new char*[ndir];
    for (i = 0 ; i < ndir ; i++)
    {
        dir_names[i] = new char[strlen(toc->dir_names[i])+1];
        strcpy(dir_names[i], toc->dir_names[i]);
    }
    int      norigdir      = toc->ndir;
    char   **origdir_names = new char*[norigdir];
    for (i = 0 ; i < norigdir ; i++)
    {
        origdir_names[i] = new char[strlen(toc->dir_names[i])+1];
        strcpy(origdir_names[i], toc->dir_names[i]);
    }
#ifdef DB_VARTYPE_SCALAR // this test can be removed after Silo-4.5-pre3 is released
    int      ndefvars = toc->ndefvars;
    char   **defvars_names = new char*[ndefvars];
    for (i = 0 ; i < ndefvars; i++)
    {
        defvars_names[i] = new char[strlen(toc->defvars_names[i])+1];
        strcpy(defvars_names[i], toc->defvars_names[i]);
    }
#endif
    int      ncurves = toc->ncurve;
    char   **curve_names = new char*[ncurves];
    for (i = 0 ; i < ncurves; i++)
    {
        curve_names[i] = new char[strlen(toc->curve_names[i])+1];
        strcpy(curve_names[i], toc->curve_names[i]);
    }

    //
    // The dbfile will probably change, so read in the meshtv_defvars and
    // meshtv_searchpath while we can.
    //
    char  *searchpath_str = NULL;
    if (strcmp(dirname, "/") == 0)
    {
	codeNameGuess = GuessCodeNameFromTopLevelVars(dbfile);

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

        bool hadVisitDefvars = false;
        if (DBInqVarExists(dbfile, "_visit_defvars"))
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

        if (!hadVisitDefvars && DBInqVarExists(dbfile, "_meshtv_defvars"))
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

        if (DBInqVarExists(dbfile, "_meshtv_searchpath"))
        {
            int    lsearchpath = DBGetVarLength(dbfile, "_meshtv_searchpath");
            if (lsearchpath > 0)
            {
                searchpath_str = new char[lsearchpath+1];
                DBReadVar(dbfile, "_meshtv_searchpath", searchpath_str);
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
    }

    //
    // Multi-meshes
    //
    for (i = 0 ; i < nmultimesh ; i++)
    {
        bool valid_var = true;
        int silo_mt = -1;
        int meshnum = 0;
        DBmultimesh *mm = GetMultimesh(dirname, multimesh_names[i]);
        if (mm)
        {
            RegisterDomainDirs(mm->meshnames, mm->nblocks, dirname);

            // Find the first non-empty mesh
            while (string(mm->meshnames[meshnum]) == "EMPTY")
            {
                meshnum++;
                if (meshnum >= mm->nblocks)
                {
                    debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                           << "\" since all its blocks are EMPTY." << endl;
                    valid_var = false;
                    break;
                }
            }

            TRY
            {
                if (valid_var)
                    silo_mt = GetMeshtype(dbfile, mm->meshnames[meshnum]);
            }
            CATCH(SiloException)
            {
                debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                       << "\" since its first non-empty block (" << mm->meshnames[meshnum]
                       << ") is invalid." << endl;
                valid_var = false;
            }
            ENDTRY
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
            AddCSGMultimesh(dirname, i, multimesh_names[i], md, mm, dbfile);
            continue;
        }

        avtMeshType mt = AVT_UNKNOWN_MESH;
        avtMeshCoordType mct = AVT_XY;
        int ndims;
        int tdims;
        int cellOrigin;
        int groupOrigin = 0;
        string xUnits, yUnits, zUnits;
        string xLabel, yLabel, zLabel;
        switch (silo_mt)
        {
          case DB_UCDMESH:
            {
                mt = AVT_UNSTRUCTURED_MESH;
                char   *realvar;
                DBfile *correctFile = dbfile;
                DetermineFileAndDirectory(mm->meshnames[meshnum], correctFile,
                                          0, realvar);
                DBucdmesh *um = DBGetUcdmesh(correctFile, realvar);
                if (um == NULL)
                {
                    debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                           << "\" since its first non-empty block (" << mm->meshnames[meshnum]
                           << ") is invalid." << endl;
                    break;
                }
                ndims = um->ndims;
                tdims = ndims; 
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
                char   *realvar;
                DBfile *correctFile = dbfile;
                DetermineFileAndDirectory(mm->meshnames[meshnum], correctFile,
                                          0, realvar);
                DBpointmesh *pm = DBGetPointmesh(correctFile, realvar);
                if (pm == NULL)
                {
                    debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                           << "\" since its first non-empty block (" << mm->meshnames[meshnum]
                           << ") is invalid." << endl;
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
            {
                mt = AVT_RECTILINEAR_MESH;
                char   *realvar;
                DBfile *correctFile = dbfile;
                DetermineFileAndDirectory(mm->meshnames[meshnum], correctFile,
                                          0, realvar);
                DBquadmesh *qm = DBGetQuadmesh(correctFile, realvar);
                if (qm == NULL)
                {
                    debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                           << "\" since its first non-empty block (" << mm->meshnames[meshnum]
                           << ") is invalid." << endl;
                    break;
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
          case DB_QUAD_CURV:
            {
                mt = AVT_CURVILINEAR_MESH;
                char   *realvar;
                DBfile *correctFile = dbfile;
                DetermineFileAndDirectory(mm->meshnames[meshnum], correctFile,
                                          0, realvar);
                DBquadmesh *qm = DBGetQuadmesh(correctFile, realvar);
                if (qm == NULL)
                {
                    debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                           << "\" since its first non-empty block (" << mm->meshnames[meshnum]
                           << ") is invalid." << endl;
                    break;
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

        char *name_w_dir = GenerateName(dirname, multimesh_names[i]);
        avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                                   mm?mm->nblocks:0, mm?mm->blockorigin:0, cellOrigin,
                                   groupOrigin, ndims, tdims, mt);
        mmd->validVariable = valid_var;
        mmd->groupTitle = "blocks";
        mmd->groupPieceName = "block";
        if (mt == AVT_UNSTRUCTURED_MESH)
            mmd->disjointElements = hasDisjointElements;
        mmd->xUnits = xUnits;
        mmd->yUnits = yUnits;
        mmd->zUnits = zUnits;
        mmd->xLabel = xLabel;
        mmd->yLabel = yLabel;
        mmd->zLabel = zLabel;
        mmd->meshCoordType = mct;
        md->Add(mmd);

        // Store off the important info about this multimesh
        // so we can match other multi-objects to it later
        StoreMultimeshInfo(dirname, i, name_w_dir, meshnum, mm);

        delete [] name_w_dir;
    }

    //
    // Quad-meshes
    //
    for (i = 0 ; i < nqmesh ; i++)
    {
        char   *realvar;
        DBfile *correctFile = dbfile;
        bool valid_var = true;

        DetermineFileAndDirectory(qmesh_names[i], correctFile, 0, realvar);
        DBquadmesh *qm = DBGetQuadmesh(correctFile, realvar);
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

        char *name_w_dir = GenerateName(dirname, qmesh_names[i]);
        avtMeshMetaData *mmd = new avtMeshMetaData(extents_to_use,
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
        md->Add(mmd);

        delete [] name_w_dir;
        DBFreeQuadmesh(qm);
    }

    //
    // Ucd-meshes
    //
    for (i = 0 ; i < nucdmesh ; i++)
    {
        char   *realvar;
        DBfile *correctFile = dbfile;
        bool valid_var = true;

        DetermineFileAndDirectory(ucdmesh_names[i], correctFile, 0, realvar);
        DBucdmesh *um = DBGetUcdmesh(correctFile, realvar);
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

        char *name_w_dir = GenerateName(dirname, ucdmesh_names[i]);
        avtMeshMetaData *mmd = new avtMeshMetaData(extents_to_use, name_w_dir,
                            1, 0, um->origin, 0, um->ndims, um->ndims,
                            AVT_UNSTRUCTURED_MESH);
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
        md->Add(mmd);

        delete [] name_w_dir;
        DBFreeUcdmesh(um);
    }

    //
    // Point meshes
    //
    for (i = 0 ; i < nptmesh ; i++)
    {
        char   *realvar;
        DBfile *correctFile = dbfile;
        bool valid_var = true;

        DetermineFileAndDirectory(ptmesh_names[i], correctFile, 0, realvar);
        DBpointmesh *pm = DBGetPointmesh(correctFile, realvar);
        if (pm == NULL)
        {
            valid_var = false;
            pm = DBAllocPointmesh(); // to fool code block below
        }

        char *name_w_dir = GenerateName(dirname, ptmesh_names[i]);
        avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir, 1, 0,pm->origin,
                                              0, pm->ndims, 0, AVT_POINT_MESH); mmd->groupTitle = "blocks";
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

        delete [] name_w_dir;
        DBFreePointmesh(pm);
    }

    //
    // Curves 
    //
    for (i = 0 ; i < ncurves; i++)
    {
        char   *realvar;
        DBfile *correctFile = dbfile;
        bool valid_var = true;

        DetermineFileAndDirectory(curve_names[i], correctFile, 0, realvar);
        DBcurve *cur = DBGetCurve(correctFile, realvar);
        if (cur == NULL)
        {
            valid_var = false;
            cur = DBAllocCurve(); // to fool code block below
        }

        char *name_w_dir = GenerateName(dirname, curve_names[i]);

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
        md->Add(cmd);

        delete [] name_w_dir;
        DBFreeCurve(cur);
    }

#ifdef DBCSG_INNER // remove after silo-4.5 is released
    //
    // Csg-meshes
    //
    for (i = 0 ; i < ncsgmesh ; i++)
    {
        char   *realvar;
        DBfile *correctFile = dbfile;
        bool valid_var = true;

        DetermineFileAndDirectory(csgmesh_names[i], correctFile, 0, realvar);

        // We want to read the header for the csg zonelist too
        // so we can serve up the "zones" of a csg mesh as "blocks"
        long mask = DBGetDataReadMask();
        DBSetDataReadMask(mask|DBCSGMZonelist|DBCSGZonelistZoneNames);
        DBcsgmesh *csgm = DBGetCsgmesh(correctFile, realvar);
        DBSetDataReadMask(mask);
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
            for (j = 0 ; j < csgm->ndims ; j++)
            {
                extents[2*j    ] = csgm->min_extents[j];
                extents[2*j + 1] = csgm->max_extents[j];
            }
            extents_to_use = extents;
        }

        char *name_w_dir = GenerateName(dirname, csgmesh_names[i]);
        avtMeshMetaData *mmd = new avtMeshMetaData(extents_to_use, name_w_dir,
                            csgm->zones->nzones, 0, csgm->origin, 0,
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
        if (csgm->zones->zonenames)
        {
            vector<string> znames;
            for (j = 0; j < csgm->zones->nzones; j++)
                znames.push_back(csgm->zones->zonenames[j]);
            mmd->blockNames = znames;
        }

        md->Add(mmd);

        delete [] name_w_dir;
        DBFreeCsgmesh(csgm);
    }
#endif

    //
    // Multi-vars
    //
    for (i = 0 ; i < nmultivar ; i++)
    {
        string meshname;
        int meshnum = 0;
        bool valid_var = true;
        DBmultivar *mv = GetMultivar(dirname, multivar_names[i]);
        if (mv != NULL)
        {

            RegisterDomainDirs(mv->varnames, mv->nvars, dirname);

            // Find the first non-empty mesh
            while (string(mv->varnames[meshnum]) == "EMPTY")
            {
                meshnum++;
                if (meshnum >= mv->nvars)
                {
                    debug1 << "Invalidating variable \"" << multivar_names[i] 
                           << "\" since all its blocks are EMPTY." << endl;
                    valid_var = false;
                    break;
                }
            }

            TRY
            {
                // NOTE: There is an explicit assumption that the corresponding
                //       multimesh has already been read.  Thus it must reside
                //       in the same directory (or a previously read one) as
                //       this variable.
                if (valid_var)
                {
                    meshname = DetermineMultiMeshForSubVariable(dbfile,
                                                                multivar_names[i],
                                                                mv->varnames,
                                                                mv->nvars, dirname);
                    debug5 << "Variable " << multivar_names[i] 
                           << " is defined on mesh " << meshname.c_str() << endl;
                }
            }
            CATCH(SiloException)
            {
                debug1 << "Invalidating var \"" << multivar_names[i] 
                       << "\" since its first non-empty block (" << mv->varnames[meshnum]
                       << ") is invalid." << endl;
                valid_var = false;
            }
            ENDTRY
        }
        else
        {
            debug1 << "Invalidating var \"" << multivar_names[i] << "\"" << endl;
            valid_var = false;
        }

        avtCentering   centering;
        bool           treatAsASCII = false;
        //
        // Get the centering and dimension information.
        //
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        string  varUnits;

        int nvals = 1;
        if (valid_var)
        {
            DetermineFileAndDirectory(mv->varnames[meshnum], correctFile, 0, realvar);

            switch (mv->vartypes[meshnum])
            {
              case DB_UCDVAR:
                {
                    DBucdvar *uv = NULL;
                    uv = DBGetUcdvar(correctFile, realvar);
                    if (uv == NULL)
                    {
                        valid_var = false;
                        break;
                    }
                    centering = (uv->centering == DB_ZONECENT ? AVT_ZONECENT 
                                                              : AVT_NODECENT);
                    nvals = uv->nvals;
                    treatAsASCII = (uv->ascii_labels);
                    if(uv->units != 0)
                        varUnits = string(uv->units);
                    DBFreeUcdvar(uv);
                }
                break;
    
              case DB_QUADVAR:
                {
                    DBquadvar *qv = DBGetQuadvar(correctFile, realvar);
                    if (qv == NULL)
                    {
                        valid_var = false;
                        break;
                    }
                    centering = (qv->align[0] == 0. ? AVT_NODECENT 
                                                    : AVT_ZONECENT);
                    nvals = qv->nvals;
                    treatAsASCII = (qv->ascii_labels);
                    if(qv->units != 0)
                        varUnits = string(qv->units);
                    DBFreeQuadvar(qv);
                }
                break;
    
              case DB_POINTVAR:
                {
                    centering = AVT_NODECENT;   // Only one possible
                    DBmeshvar *pv = DBGetPointvar(correctFile, realvar);
                    if (pv == NULL)
                    {
                        valid_var = false;
                        break;
                    }
                    nvals = pv->nvals;
                    treatAsASCII = (pv->ascii_labels);
                    if(pv->units != 0)
                        varUnits = string(pv->units);
                    DBFreeMeshvar(pv);
                }
                break;


#ifdef DBCSG_INNER // remove after silo-4.5 is released
              case DB_CSGVAR:
                {
                    DBcsgvar *csgv = DBGetCsgvar(correctFile, realvar);
                    centering = csgv->centering == DB_BNDCENT ? AVT_NODECENT
                                                              : AVT_ZONECENT;
                    if (csgv == NULL)
                    {
                        valid_var = false;
                        break;
                    }
                    nvals = csgv->nvals;
                    treatAsASCII = (csgv->ascii_labels);
                    if(csgv->units != 0)
                        varUnits = string(csgv->units);
                    DBFreeCsgvar(csgv);
                }
                break;
#endif
              default:
                EXCEPTION1(InvalidVariableException, multivar_names[i]);
                // Compiler complains about a break here.
            };
        }

        char *name_w_dir = GenerateName(dirname, multivar_names[i]);
        if (nvals == 1)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                       meshname, centering);
            smd->validVariable = valid_var;
            smd->treatAsASCII = treatAsASCII;
            if(varUnits != "")
            {
                smd->hasUnits = true;
                smd->units = varUnits;
            }
	    if (codeNameGuess == "Ale3d" && string(multivar_names[i]) == "rlxstat")
	        AddAle3drlxstatEnumerationInfo(smd);
            md->Add(smd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                             meshname, centering, nvals);
            vmd->validVariable = valid_var;
            if(varUnits != "")
            {
                vmd->hasUnits = true;
                vmd->units = varUnits;
            }
            md->Add(vmd);
        }
        delete [] name_w_dir;
    }

    //
    // Quadvars
    //
    for (i = 0 ; i < nqvar ; i++)
    {
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        bool valid_var = true;
        DetermineFileAndDirectory(qvar_names[i], correctFile, 0, realvar);
        DBquadvar *qv = DBGetQuadvar(correctFile, realvar);
        if (qv == NULL)
        {
            valid_var = false;
            qv = DBAllocQuadvar();
        }

        char meshname[256];
        DBInqMeshname(correctFile, realvar, meshname);

        //
        // Get the centering information.
        //
        avtCentering   centering = (qv->align[0] == 0. ? AVT_NODECENT :
                                                         AVT_ZONECENT);
        
        //
        // Get the dimension of the variable.
        //
        char *name_w_dir = GenerateName(dirname, qvar_names[i]);
        char *meshname_w_dir = GenerateName(dirname, meshname);
        if (qv->nvals == 1)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                    meshname_w_dir, centering);
            smd->treatAsASCII = (qv->ascii_labels);
            smd->validVariable = valid_var;
            if(qv->units != 0)
            {
                smd->hasUnits = true;
                smd->units = string(qv->units);
            }
            md->Add(smd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                         meshname_w_dir, centering, qv->nvals);
            vmd->validVariable = valid_var;
            if(qv->units != 0)
            {
                vmd->hasUnits = true;
                vmd->units = string(qv->units);
            }
            md->Add(vmd);
        }
        delete [] name_w_dir;
        delete [] meshname_w_dir;
        DBFreeQuadvar(qv);
    }

    //
    // Ucdvars
    //
    for (i = 0 ; i < nucdvar ; i++)
    {
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        bool valid_var = true;
        DetermineFileAndDirectory(ucdvar_names[i], correctFile, 0, realvar);
        DBucdvar *uv = DBGetUcdvar(correctFile, realvar);
        if (uv == NULL)
        {
            valid_var = false;
            uv = DBAllocUcdvar();
        }

        char meshname[256];
        DBInqMeshname(correctFile, realvar, meshname);

        //
        // Get the centering information.
        //
        avtCentering centering = (uv->centering == DB_ZONECENT ? AVT_ZONECENT
                                                               : AVT_NODECENT);

        //
        // Get the dimension of the variable.
        //
        char *name_w_dir = GenerateName(dirname, ucdvar_names[i]);
        char *meshname_w_dir = GenerateName(dirname, meshname);
        if (uv->nvals == 1)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                    meshname_w_dir, centering);
            smd->validVariable = valid_var;
            smd->treatAsASCII = (uv->ascii_labels);
            if(uv->units != 0)
            {
                smd->hasUnits = true;
                smd->units = string(uv->units);
            }
            md->Add(smd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                         meshname_w_dir, centering, uv->nvals);
            vmd->validVariable = valid_var;
            if(uv->units != 0)
            {
                vmd->hasUnits = true;
                vmd->units = string(uv->units);
            }
            md->Add(vmd);
        }
        delete [] name_w_dir;
        delete [] meshname_w_dir;
        DBFreeUcdvar(uv);
    }

    //
    // Point vars
    //
    for (i = 0 ; i < nptvar ; i++)
    {
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        bool valid_var = true;
        DetermineFileAndDirectory(ptvar_names[i], correctFile, 0, realvar);
        DBmeshvar *pv = DBGetPointvar(correctFile, realvar);
        if (pv == NULL)
        {
            valid_var = false;
            pv = DBAllocMeshvar();
        }

        char meshname[256];
        DBInqMeshname(correctFile, realvar, meshname);

        //
        // Get the dimension of the variable.
        //
        char *name_w_dir = GenerateName(dirname, ptvar_names[i]);
        char *meshname_w_dir = GenerateName(dirname, meshname);
        if (pv->nvals == 1)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                meshname_w_dir, AVT_NODECENT);
            smd->treatAsASCII = (pv->ascii_labels);
            smd->validVariable = valid_var;
            if(pv->units != 0)
            {
                smd->hasUnits = true;
                smd->units = string(pv->units);
            }
            md->Add(smd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                      meshname_w_dir, AVT_NODECENT, pv->nvals);
            vmd->validVariable = valid_var;
            if(pv->units != 0)
            {
                vmd->hasUnits = true;
                vmd->units = string(pv->units);
            }
            md->Add(vmd);
        }
        delete [] name_w_dir;
        delete [] meshname_w_dir;
        DBFreeMeshvar(pv);
    }

    //
    // Csgvars
    //
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    for (i = 0 ; i < ncsgvar ; i++)
    {
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        bool valid_var = true;
        DetermineFileAndDirectory(csgvar_names[i], correctFile, 0, realvar);
        DBcsgvar *csgv = DBGetCsgvar(correctFile, realvar);
        if (csgv == NULL)
        {
            valid_var = false;
            csgv = DBAllocCsgvar();
        }

        char meshname[256];
        DBInqMeshname(correctFile, realvar, meshname);

        //
        // Get the centering information.
        //
//#warning USING AVT_NODECENT FOR DB_BNDCENT
        avtCentering centering = (csgv->centering == DB_BNDCENT ? AVT_NODECENT
                                                               : AVT_ZONECENT);

        //
        // Get the dimension of the variable.
        //
        char *name_w_dir = GenerateName(dirname, csgvar_names[i]);
        char *meshname_w_dir = GenerateName(dirname, meshname);
        if (csgv->nvals == 1)
        {
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                    meshname_w_dir, centering);
            smd->treatAsASCII = (csgv->ascii_labels);
            smd->validVariable = valid_var;
            if(csgv->units != 0)
            {
                smd->hasUnits = true;
                smd->units = string(csgv->units);
            }
            md->Add(smd);
        }
        else
        {
            avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                         meshname_w_dir, centering, csgv->nvals);
            vmd->validVariable = valid_var;
            if(csgv->units != 0)
            {
                vmd->hasUnits = true;
                vmd->units = string(csgv->units);
            }
            md->Add(vmd);
        }
        delete [] name_w_dir;
        delete [] meshname_w_dir;
        DBFreeCsgvar(csgv);
    }
#endif

    //
    // Materials
    //
    for (i = 0 ; i < nmat ; i++)
    {
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        bool valid_var = true;
        DetermineFileAndDirectory(mat_names[i], correctFile, 0, realvar);
        DBmaterial *mat = DBGetMaterial(correctFile, realvar);
        if (mat == NULL)
        {
            valid_var = false;
            mat = DBAllocMaterial();
        }

        char meshname[256];
        DBInqMeshname(correctFile, realvar, meshname);

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
            if (mat->matnames == NULL)
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
#ifdef DBOPT_MATCOLORS
            if (mat->matcolors)
            {
                if (mat->matcolors[j] && mat->matcolors[j][0])
                    matcolors.push_back(mat->matcolors[j]);
                else
                    matcolors.push_back("");
            }
#endif
        }

        char *name_w_dir = GenerateName(dirname, mat_names[i]);
        char *meshname_w_dir = GenerateName(dirname, meshname);
        avtMaterialMetaData *mmd;
        if (matcolors.size())
            mmd = new avtMaterialMetaData(name_w_dir, meshname_w_dir,
                                          mat->nmat, matnames, matcolors);
        else
            mmd = new avtMaterialMetaData(name_w_dir, meshname_w_dir,
                                          mat->nmat, matnames);
        mmd->validVariable = valid_var;
        md->Add(mmd);
//#warning FIX MATERIAL PROBLEMS FOR CSG

        delete [] name_w_dir;
        delete [] meshname_w_dir;
        DBFreeMaterial(mat);
    }

    //
    // Multi-mats
    //
    for (i = 0 ; i < nmultimat ; i++)
    {
        bool valid_var = true;
        DBmultimat *mm = GetMultimat(dirname, multimat_names[i]);
        if (mm == NULL)
        {
            valid_var = false;
            mm = DBAllocMultimat(0);
        }
        RegisterDomainDirs(mm->matnames, mm->nmats, dirname);

        // Find the first non-empty mesh
        int meshnum = 0;
        while (string(mm->matnames[meshnum]) == "EMPTY")
        {
            meshnum++;
            if (meshnum >= mm->nmats)
            {
                debug1 << "Invalidating material \"" << multimat_names[i] 
                       << "\" since all its blocks are EMPTY." << endl;
                valid_var = false;
                break;
            }
        }

        char *material = mm->matnames[meshnum];

        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        DBmaterial *mat = NULL;

        if (valid_var)
        {
            DetermineFileAndDirectory(material, correctFile, 0, realvar);
            mat = DBGetMaterial(correctFile, realvar);
        }

        if (mat == NULL)
        {
            debug1 << "Invalidating material \"" << multimat_names[i] 
                   << "\" since its first non-empty block (" << material
                   << ") is invalid." << endl;
            valid_var = false;
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
            for (j = 0 ; j < mat->nmat ; j++)
            {
                char *num = NULL;
                int dlen = int(log10(float(mat->matnos[j]+1))) + 1;
                if (mat->matnames == NULL)
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

#ifdef DBOPT_MATCOLORS
                if (mat->matcolors)
                {
                    if (mat->matcolors[j] && mat->matcolors[j][0])
                        matcolors.push_back(mat->matcolors[j]);
                    else
                        matcolors.push_back("");
                }
#endif
            }

            TRY
            {
                meshname = DetermineMultiMeshForSubVariable(dbfile,
                                                            multimat_names[i],
                                                            mm->matnames,
                                                            mm->nmats, dirname);
                debug5 << "Material " << multimat_names[i]<< " is defined on mesh "
                       << meshname.c_str() << endl;
            }
            CATCH(SiloException)
            {
                debug1 << "Giving up on var \"" << multimat_names[i] 
                       << "\" since its first non-empty block (" << material
                       << ") is invalid." << endl;
                valid_var = false;
            }
            ENDTRY
        }

        char *name_w_dir = GenerateName(dirname, multimat_names[i]);
        avtMaterialMetaData *mmd;
        if (matcolors.size())
            mmd = new avtMaterialMetaData(name_w_dir, meshname,
                                          mat ? mat->nmat : 0, matnames,
                                          matcolors);
        else
            mmd = new avtMaterialMetaData(name_w_dir, meshname,
                                          mat ? mat->nmat : 0, matnames);

        mmd->validVariable = valid_var;
        md->Add(mmd);
//#warning FIX MATERIAL PROBLEMS FOR CSG 

        delete [] name_w_dir;
        DBFreeMaterial(mat);
    }

    //
    // Species
    //
    for (i = 0 ; i < nmatspecies ; i++)
    {
        char   *realvar = NULL;
        DBfile *correctFile = dbfile;
        bool valid_var = true;
        DetermineFileAndDirectory(matspecies_names[i], correctFile, 0, realvar);

        DBmatspecies *spec = DBGetMatspecies(correctFile, realvar);
        if (spec == NULL)
        {
            valid_var = false;
            spec = DBAllocMatspecies();
        }

        char meshname[256];
        GetMeshname(dbfile, spec->matname, meshname);

        vector<int>   numSpecies;
        vector<vector<string> > speciesNames;
        for (j = 0 ; j < spec->nmat ; j++)
        {
            numSpecies.push_back(spec->nmatspec[j]);
            vector<string>  tmp_string_vector;

            //
            // Species do not currently have names, so just use their index.
            //
            for (k = 0 ; k < spec->nmatspec[j] ; k++)
            {
                char num[16];
                sprintf(num, "%d", k+1);
                tmp_string_vector.push_back(num);
            }
            speciesNames.push_back(tmp_string_vector);
        }
        char *name_w_dir = GenerateName(dirname, matspecies_names[i]);
        char *meshname_w_dir = GenerateName(dirname, meshname);
        avtSpeciesMetaData *smd = new avtSpeciesMetaData(name_w_dir,
                                  meshname_w_dir, spec->matname, spec->nmat,
                                  numSpecies, speciesNames);
        md->Add(smd);

        delete [] name_w_dir;
        delete [] meshname_w_dir;
        DBFreeMatspecies(spec);
    }

    //
    // Multi-species
    //
    for (i = 0 ; i < nmultimatspecies ; i++)
    {
        DBmultimatspecies *ms = GetMultimatspec(dirname,
                                                     multimatspecies_names[i]);
        if (ms == NULL)
            EXCEPTION1(InvalidVariableException, multimatspecies_names[i]);

        RegisterDomainDirs(ms->specnames, ms->nspec, dirname);

        // Find the first non-empty mesh
        int meshnum = 0;
        bool valid_var = true;
        while (string(ms->specnames[meshnum]) == "EMPTY")
        {
            meshnum++;
            if (meshnum >= ms->nspec)
            {
                debug1 << "Invalidating species \"" << multimatspecies_names[i] 
                       << "\" since all its blocks are EMPTY." << endl;
                valid_var = false;
                break;
            }
        }

        string meshname;
        DBmatspecies *spec = NULL;

        if (valid_var)
        {
            // get the associated multimat

            // We can only get this "matname" using GetComponent.  It it not
            // yet a part of the DBmultimatspec structure, so this is the only
            // way we can get this info.
            char *multimatName = (char *) GetComponent(dbfile,
                                              multimatspecies_names[i], "matname");

            // get the multimesh for the multimat
            DBmultimat *mm = GetMultimat(dirname, multimatName);
            if (mm == NULL)
                EXCEPTION1(InvalidVariableException, multimatspecies_names[i]);
            char *material = mm->matnames[meshnum];

            TRY
            {
                meshname = DetermineMultiMeshForSubVariable(dbfile,
                                                          multimatspecies_names[i],
                                                          mm->matnames,
                                                          ms->nspec, dirname);
            }
            CATCH(SiloException)
            {
                debug1 << "Giving up on var \"" << multimatspecies_names[i]
                       << "\" since its first non-empty block (" << material
                       << ") is invalid." << endl;
                valid_var = false;
            }
            ENDTRY

            // get the species info
            char *species = ms->specnames[meshnum];

            char   *realvar = NULL;
            DBfile *correctFile = dbfile;
            DetermineFileAndDirectory(species, correctFile, 0, realvar);

            DBShowErrors(DB_NONE, NULL);
            spec = DBGetMatspecies(correctFile, realvar);
            DBShowErrors(DB_ALL, ExceptionGenerator);
            if (spec == NULL)
            {
                debug1 << "Giving up on species \"" << multimatspecies_names[i]
                       << "\" since its first non-empty block (" << species
                       << ") is invalid." << endl;
                valid_var = false;
            }
        }

        vector<int>              numSpecies;
        vector< vector<string> > speciesNames;
        if (valid_var)
        {
            for (j = 0 ; j < spec->nmat ; j++)
            {
                numSpecies.push_back(spec->nmatspec[j]);
                vector<string>  tmp_string_vector;
                for (k = 0 ; k < spec->nmatspec[j] ; k++)
                {
                    char num[16];
                    sprintf(num, "%d", k+1);
                    tmp_string_vector.push_back(num);
                }
                speciesNames.push_back(tmp_string_vector);
            }
        }
        char *name_w_dir = GenerateName(dirname, multimatspecies_names[i]);
        avtSpeciesMetaData *smd;
        if (valid_var)
            smd = new avtSpeciesMetaData(name_w_dir,
                                         meshname, spec->matname, spec->nmat,
                                         numSpecies, speciesNames);
        else
            smd = new avtSpeciesMetaData(name_w_dir, "", "", 0,
                                         numSpecies, speciesNames);
        smd->validVariable = valid_var;
        md->Add(smd);

        delete [] name_w_dir;
        DBFreeMatspecies(spec);
    }

    //
    // Add defvars objects (like _visit_defvars except a real Silo object)
    //
#ifdef DB_VARTYPE_SCALAR // this test can be removed after Silo-4.5-pre3 is released
    for (i = 0; i < ndefvars; i++)
    {
        DBdefvars *defv = DBGetDefvars(dbfile, defvars_names[i]); 
        if (defv == NULL)
            EXCEPTION1(InvalidVariableException, defvars_names[i]);

        for (int j = 0; j < defv->ndefs; j++)
        {
            Expression::ExprType vartype = Expression::Unknown;
            switch (defv->types[j])
            {
                case DB_VARTYPE_SCALAR:   vartype = Expression::ScalarMeshVar; break;
                case DB_VARTYPE_VECTOR:   vartype = Expression::VectorMeshVar; break;
                case DB_VARTYPE_TENSOR:   vartype = Expression::TensorMeshVar; break;
#ifdef DB_VARTYPE_ARRAY // this test can be removed after Silo-4.5-pre3 is released
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
                continue;
            }

            Expression expr;
                expr.SetName(defv->names[j]);
                expr.SetDefinition(defv->defns[j]);
                expr.SetType(vartype);
            md->AddExpression(&expr);
        }
        DBFreeDefvars(defv);
    }
#endif

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
        dir_names = new char*[ndir];
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
                ndir++;
            }
        }
          
        delete [] searchpath_str;
    }

    //
    // Call recursively on the sub-directories.
    //
    for (i = 0 ; i < ndir ; i++)
    {
        char path[1024];
        int length = strlen(dirname);
        if (length > 0 && dirname[length-1] != '/')
        {
            sprintf(path, "%s/%s", dirname, dir_names[i]);
        }
        else
        {
            sprintf(path, "%s%s", dirname, dir_names[i]);
        }
        if (!ShouldGoToDir(path))
        {
            continue;
        }
        DBSetDir(dbfile, dir_names[i]);
        ReadDir(dbfile, path, md);
        DBSetDir(dbfile, "..");
    }

    for (i = 0 ; i < nmultimesh ; i++)
    {
        delete [] multimesh_names[i];
    }
    delete [] multimesh_names;
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    for (i = 0 ; i < ncsgmesh ; i++)
    {
        delete [] csgmesh_names[i];
    }
    delete [] csgmesh_names;
#endif
    for (i = 0 ; i < nqmesh ; i++)
    {
        delete [] qmesh_names[i];
    }
    delete [] qmesh_names;
    for (i = 0 ; i < nucdmesh ; i++)
    {
        delete [] ucdmesh_names[i];
    }
    delete [] ucdmesh_names;
    for (i = 0 ; i < nptmesh ; i++)
    {
        delete [] ptmesh_names[i];
    }
    delete [] ptmesh_names;
    for (i = 0 ; i < nmultivar ; i++)
    {
        delete [] multivar_names[i];
    }
    delete [] multivar_names;
    for (i = 0 ; i < nqvar ; i++)
    {
        delete [] qvar_names[i];
    }
    delete [] qvar_names;
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    for (i = 0 ; i < ncsgvar ; i++)
    {
        delete [] csgvar_names[i];
    }
    delete [] csgvar_names;
#endif
    for (i = 0 ; i < nucdvar ; i++)
    {
        delete [] ucdvar_names[i];
    }
    delete [] ucdvar_names;
    for (i = 0 ; i < nptvar ; i++)
    {
        delete [] ptvar_names[i];
    }
    delete [] ptvar_names;
    for (i = 0 ; i < nmat ; i++)
    {
        delete [] mat_names[i];
    }
    delete [] mat_names;
    for (i = 0 ; i < nmultimat ; i++)
    {
        delete [] multimat_names[i];
    }
    delete [] multimat_names;
    for (i = 0 ; i < nmatspecies ; i++)
    {
        delete [] matspecies_names[i];
    }
    delete [] matspecies_names;
    for (i = 0 ; i < nmultimatspecies ; i++)
    {
        delete [] multimatspecies_names[i];
    }
    delete [] multimatspecies_names;
    for (i = 0 ; i < ndir ; i++)
    {
        delete [] dir_names[i];
    }
    delete [] dir_names;
    for (i = 0 ; i < norigdir ; i++)
    {
        delete [] origdir_names[i];
    }
    delete [] origdir_names;
#ifdef DB_VARTYPE_SCALAR // this test can be removed after Silo-4.5-pre3 is released
    for (i = 0 ; i < ndefvars; i++)
    {
        delete [] defvars_names[i];
    }
    delete [] defvars_names;
#endif
    for (i = 0 ; i < ncurves; i++)
    {
        delete [] curve_names[i];
    }
    delete [] curve_names;
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
    
    //
    // Broadcast Group Info
    //
    int haveGroups = groupInfo.haveGroups;
    BroadcastInt(haveGroups);
    groupInfo.haveGroups = haveGroups;
    BroadcastInt(groupInfo.ndomains);
    BroadcastInt(groupInfo.numgroups);
    BroadcastIntVector(groupInfo.ids,  rank);
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
// ****************************************************************************
void
avtSiloFileFormat::StoreMultimeshInfo(const char *const dirname, int which_mm,
const char *const name_w_dir, int meshnum, const DBmultimesh *const mm)
{
    actualMeshName.push_back(name_w_dir);
    firstSubMesh.push_back((meshnum < mm->nblocks) ?
                            mm->meshnames[meshnum] : "");
    blocksForMesh.push_back(mm->nblocks);
    allSubMeshDirs.push_back(vector<string>());
    for (int j=0; j<mm->nblocks; j++)
    {
        string dir,var;
        SplitDirVarName(mm->meshnames[j], dirname, dir,var);
        if (j==meshnum)
            firstSubMeshVarName.push_back(var);
        allSubMeshDirs[which_mm].push_back(dir);
    }
    if (meshnum >= mm->nblocks)
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

    if (groupInfo.haveGroups)
        md->AddGroupInformation(groupInfo.numgroups, groupInfo.ndomains,
                                groupInfo.ids);
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
// ****************************************************************************

void
avtSiloFileFormat::GetConnectivityAndGroupInformation(DBfile *dbfile, 
                                                      bool force)
{
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

    //
    // Communicate processor 0's information to the rest of the processors.
    //
    MPI_Bcast(&ndomains, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (ndomains != -1)
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
    if (ndomains > 0 && !avtDatabase::OnlyServeUpMetaData())
    {
        avtCurvilinearDomainBoundaries *dbi =
                                          new avtCurvilinearDomainBoundaries();
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
        canDoDynamicLoadBalancing = false;
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

            DBSetDir(dbfile, "/");
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
            numGroups = groupMap.size();
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
// ****************************************************************************


void
avtSiloFileFormat::FindStandardConnectivity(DBfile *dbfile, int &ndomains,
            int *&nneighbors, int *&extents, int &lneighbors, int *&neighbors,
            int &numGroups, int *&groupIds, bool needConnectivityInfo,
            bool needGroupInfo)
{
    bool packed_conn_info = DBInqVarExists(dbfile,"NumDecomp_pack") != 0;

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

    if (!packed_conn_info) // use standard connectivity info
    {
        debug1 << "avtSiloFileFormat: using standard connectivity info" <<endl;
        for (int j = 0 ; j < ndomains ; j++)
        {
            char dirname[256];
            if (j > 0)
                sprintf(dirname, "../Domain_%d", j);
            else
                sprintf(dirname, "Domain_%d", j);
            if (DBSetDir(dbfile, dirname))
            {
                ndomains = -1;
                numGroups = -1;
                break;
            }

            if (needConnectivityInfo)
            {
                DBReadVar(dbfile, "Extents", &extents[j*6]);
                DBReadVar(dbfile, "NumNeighbors", &nneighbors[j]);
                lneighbors += nneighbors[j] * 11;
            }

            if (needGroupInfo)
            {
                DBReadVar(dbfile, "BlockNum", &(groupIds[j]));
            }

    }

        DBSetDir(dbfile, "..");

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
    else // used packed connectivity info
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

    // Get the MultiMeshAdjacency object
    DBmultimeshadj *mmadj_obj = DBGetMultimeshadj(dbfile,
                                                  "Domain_Decomposition",
                                                  0,NULL);
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
                "VisIt only supports MultiMeshadj objects with "
                "structured meshes");
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

        lneighbors = mmadj_obj->lneighbors * 11;
        neighbors  = new int[lneighbors];

        int *extents_ptr = extents;
        int *neighbors_ptr = neighbors;

        int idx = 0;
        for( i =0; i < ndomains; i++)
        {
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
            neighbors = new int[lneighbors];
            int index = 0;
            for (int j = 0 ; j < ndomains ; j++)
            {
                for (int k = 0 ; k < nneighbors[j] ; k++)
                {
                    char neighborname[256];
                    sprintf(neighborname, "gmap%d/neighbor%d",j,k);
                    int neighbor_info[512];  // We only want 11, but it can
                                             // be as big as 400. 
                    DBReadVar(dbfile, neighborname, neighbor_info);
                    for (int l = 0 ; l < 11 ; l++)
                    {
                        neighbors[index+l] = neighbor_info[l];
                    }
                    index += 11;
                }
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
// ****************************************************************************
void
avtSiloFileFormat::AddCSGMultimesh(const char *const dirname, int which_mm,
    const char *const multimesh_name, avtDatabaseMetaData *md,
    const DBmultimesh *const mm, DBfile *dbfile)
{
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

    long mask = DBGetDataReadMask();
    DBSetDataReadMask(mask|DBCSGMZonelist|DBCSGZonelistZoneNames);

    for (i = 0; i < mm->nblocks; i++)
    {
        if (string(mm->meshnames[i]) == "EMPTY")
            continue;

        char   *realvar;
        DBfile *correctFile = dbfile;
        DetermineFileAndDirectory(mm->meshnames[i], correctFile, 0, realvar);
        DBcsgmesh *csgm = DBGetCsgmesh(correctFile, realvar);
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
        }

        DBFreeCsgmesh(csgm);

    }
    DBSetDataReadMask(mask);


    // a value for meshnum of -1 at this point indicates
    // no non-EMPTY blocks found
    if (meshnum != -1)
    {

        char *name_w_dir = GenerateName(dirname, multimesh_name);
        avtMeshMetaData *mmd = new avtMeshMetaData(extents_to_use, name_w_dir,
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
        StoreMultimeshInfo(dirname, which_mm, name_w_dir, meshnum, mm);

        delete [] name_w_dir;
    }
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
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetVar(int domain, const char *v)
{
    // for CSG meshes, each domain is a csgregion and a group of regions
    // forms a visit "domain". So, we need to re-map the domain id
    metadata->ConvertCSGDomainToBlockAndRegion(v, &domain, 0);

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
    DBmultivar *mv = QueryMultivar("", var);
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

    char  *varLocation = NULL;
    if (type == DB_MULTIVAR)
    {
        if (mv == NULL)
            mv = GetMultivar("", var);
        if (mv == NULL)
            EXCEPTION1(InvalidVariableException, var);
        if (localdomain < 0 || localdomain >= mv->nvars)
        {
            EXCEPTION2(BadDomainException, localdomain, mv->nvars);
        }
        type = mv->vartypes[localdomain];
        varLocation = new char[strlen(mv->varnames[localdomain])+1];
        strcpy(varLocation, mv->varnames[localdomain]);
    }
    else
    {
        if (domain != 0)
        {
            EXCEPTION2(BadDomainException, domain, 1);
        }
        varLocation = new char[strlen(var)+1];
        strcpy(varLocation, var);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    char   *directory_var = NULL;
    const char *varDirname = StringHelpers::Dirname(var);
    bool allocated_directory_var;
    DetermineFileAndDirectory(varLocation, domain_file, varDirname, directory_var,
        &allocated_directory_var);

    //
    // We only need to worry about quadvars, ucdvars, and pointvars, since we
    // have reduced the multivar case to one of those.
    //
    vtkDataArray *rv = NULL;
    if (type == DB_UCDVAR)
    {
        rv = GetUcdVar(domain_file, directory_var, v, domain);
    }
    else if (type == DB_QUADVAR)
    {
        rv = GetQuadVar(domain_file, directory_var, v, domain);
    }
    else if (type == DB_POINTVAR)
    {
        rv = GetPointVar(domain_file, directory_var);
    }
    else if (type == DB_CSGVAR)
    {
        rv = GetCsgVar(domain_file, directory_var);
    }

    //
    // This may be leaked if an exception is thrown after it is allocated.
    // I'll live.
    //
    delete [] varLocation;
    if (allocated_directory_var)
        delete [] directory_var;

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
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetVectorVar(int domain, const char *v)
{
    // for CSG meshes, each domain is a csgregion and a group of regions
    // forms a visit "domain". So, we need to re-map the domain id
    metadata->ConvertCSGDomainToBlockAndRegion(v, &domain, 0);

    debug5 << "Reading in vector variable " << v << ", domain " << domain
           << endl;
    
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
    DBmultivar *mv = QueryMultivar("", var);
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

    char  *varLocation = NULL;
    if (type == DB_MULTIVAR)
    {
        if (mv == NULL)
            mv = GetMultivar("", var);
        if (mv == NULL)
            EXCEPTION1(InvalidVariableException, var);
        if (localdomain < 0 || localdomain >= mv->nvars)
        {
            EXCEPTION2(BadDomainException, localdomain, mv->nvars);
        }
        type = mv->vartypes[localdomain];
        varLocation = new char[strlen(mv->varnames[localdomain])+1];
        strcpy(varLocation, mv->varnames[localdomain]);
    }
    else
    {
        if (domain != 0)
        {
            EXCEPTION2(BadDomainException, domain, 1);
        }
        varLocation = new char[strlen(var)+1];
        strcpy(varLocation, var);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    char   *directory_var = NULL;
    const char *varDirname = StringHelpers::Dirname(var);
    bool allocated_directory_var;
    DetermineFileAndDirectory(varLocation, domain_file, varDirname, directory_var,
        &allocated_directory_var);

    //
    // We only need to worry about quadvars, ucdvars, and pointvars, since we
    // have reduced the multivar case to one of those.
    //
    vtkDataArray *rv = NULL;
    if (type == DB_UCDVAR)
    {
        rv = GetUcdVectorVar(domain_file, directory_var, v, domain);
    }
    else if (type == DB_QUADVAR)
    {
        rv = GetQuadVectorVar(domain_file, directory_var, v, domain);
    }
    else if (type == DB_POINTVAR)
    {
        rv = GetPointVectorVar(domain_file, directory_var);
    }
    else if (type == DB_CSGVAR)
    {
        rv = GetCsgVectorVar(domain_file, directory_var);
    }

    //
    // This may be leaked if an exception is thrown after it is allocated.
    // I'll live.
    //
    delete [] varLocation;
    if (allocated_directory_var)
        delete [] directory_var;

    return rv;
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
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray   *vectors = vtkFloatArray::New();
    vectors->SetNumberOfComponents(3);

    //
    // Handle cases where we need to remove values for zone types we don't
    // understand
    //
    float *vals[3];
    vals[0] = uv->vals[0];
    vals[1] = uv->vals[1];
    if (uv->nvals == 3)
       vals[2] = uv->vals[2];
    int numSkipped = 0;
    if (uv->centering == DB_ZONECENT && metadata != NULL)
    {
        string meshName = metadata->MeshForVar(tvn);
        vector<int> zonesRangesToSkip = arbMeshZoneRangesToSkip[meshName];
        if (zonesRangesToSkip.size() > 0)
        {
            numSkipped = ComputeNumZonesSkipped(zonesRangesToSkip);
            vals[0] = new float[uv->nels - numSkipped];
            vals[1] = new float[uv->nels - numSkipped];
            if (uv->nvals == 3)
                vals[2] = new float[uv->nels - numSkipped];

            RemoveValuesForSkippedZones(zonesRangesToSkip,
                uv->vals[0], uv->nels, vals[0]);
            RemoveValuesForSkippedZones(zonesRangesToSkip,
                uv->vals[1], uv->nels, vals[1]);
            if (uv->nvals == 3)
                RemoveValuesForSkippedZones(zonesRangesToSkip,
                    uv->vals[2], uv->nels, vals[2]);
        }
    }

    vectors->SetNumberOfTuples(uv->nels - numSkipped);
    for (int i = 0 ; i < uv->nels - numSkipped; i++)
    {
        float v3 = (uv->nvals == 3 ? vals[2][i] : 0.);
        vectors->SetTuple3(i, vals[0][i], vals[1][i], v3);
    }

    if (vals[0] != uv->vals[0])
    {
        delete [] vals[0];
        delete [] vals[1];
        if (uv->nvals == 3)
            delete [] vals[2];
    }

    DBFreeUcdvar(uv);

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

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkDataArray *vectors;
    if (qv->datatype == DB_DOUBLE)
        vectors = vtkDoubleArray::New();
    else
        vectors = vtkFloatArray::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetNumberOfTuples(qv->nels);
    if (qv->datatype == DB_DOUBLE)
    {
        double *v1 = (double *) qv->vals[0];
        double *v2 = (double *) qv->vals[1];
        double *v3 = (double *) (qv->nvals == 3 ? qv->vals[2] : 0);
        for (int i = 0 ; i < qv->nels ; i++)
            vectors->SetTuple3(i, v1[i], v2[i], v3 ? v3[i] : 0.);
    }
    else
    {
        for (int i = 0 ; i < qv->nels ; i++)
        {
            float v3 = (qv->nvals == 3 ? qv->vals[2][i] : 0.);
            vectors->SetTuple3(i, qv->vals[0][i], qv->vals[1][i], v3);
        }
    }

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

    vtkFloatArray   *vectors = vtkFloatArray::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetNumberOfTuples(mv->nels);
    for (int i = 0 ; i < mv->nels ; i++)
    {
        float v3 = (mv->nvals == 3 ? mv->vals[2][i] : 0.);
        vectors->SetTuple3(i, mv->vals[0][i], mv->vals[1][i], v3);
    }

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
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetMesh(int domain, const char *m)
{
    // for CSG meshes, each domain is a csgregion and a group of regions
    // forms a visit "domain". So, we need to re-map the domain id
    metadata->ConvertCSGDomainToBlockAndRegion(m, &domain, 0);

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
    DBmultimesh *mm = QueryMultimesh("", mesh);
    int type;
    if (mm != NULL)
        type = DB_MULTIMESH;
    else
        type = DBInqVarType(dbfile, mesh);

    //
    // Sort out the bad cases.
    //
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    if (type != DB_UCDMESH && type != DB_QUADMESH && type != DB_QUAD_CURV &&
        type != DB_QUAD_RECT && type != DB_POINTMESH && type != DB_MULTIMESH &&
        type != DB_CSGMESH && type != DB_CURVE)
#else
    if (type != DB_UCDMESH && type != DB_QUADMESH && type != DB_QUAD_CURV &&
        type != DB_QUAD_RECT && type != DB_POINTMESH && type != DB_MULTIMESH &&
        type != DB_CURVE)
#endif
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    char  *meshLocation = NULL;
    if (type == DB_MULTIMESH)
    {
        if (mm == NULL)
           mm = GetMultimesh("", mesh);
        if (mm == NULL)
            EXCEPTION1(InvalidVariableException, mesh);
        if (domain < 0 || domain >= mm->nblocks)
        {
            EXCEPTION2(BadDomainException, domain, mm->nblocks);
        }
        type = mm->meshtypes[domain];
        meshLocation = new char[strlen(mm->meshnames[domain])+1];
        strcpy(meshLocation, mm->meshnames[domain]);
    }
    else
    {
#ifdef DBCSG_INNER // remove after silo-4.5 is released
        if (domain != 0 && type != DB_CSGMESH)
#else
        if (domain != 0)
#endif
        {
            EXCEPTION2(BadDomainException, domain, 1);
        }
        meshLocation = new char[strlen(mesh)+1];
        strcpy(meshLocation, mesh);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    char   *directory_mesh = NULL;
    const char *meshDirname = StringHelpers::Dirname(mesh);
    bool allocated_directory_mesh;
    DetermineFileAndDirectory(meshLocation, domain_file, meshDirname, directory_mesh,
        &allocated_directory_mesh);

    //
    // We only need to worry about quadmeshes, ucdmeshes, and pointmeshes,
    // since we have reduced the multimesh case to one of those.
    //
    vtkDataSet *rv = NULL;
    if (type == DB_UCDMESH)
    {
        rv = GetUnstructuredMesh(domain_file, directory_mesh, domain, m);
    }
    else if (type == DB_QUADMESH || type == DB_QUAD_RECT || type==DB_QUAD_CURV)
    {
        rv = GetQuadMesh(domain_file, directory_mesh, domain);
    }
    else if (type == DB_POINTMESH)
    {
        rv = GetPointMesh(domain_file, directory_mesh);
    }
#ifdef DBCSG_INNER // remove after silo-4.5 is released
    else if (type == DB_CSGMESH)
    {
        rv = GetCSGMesh(domain_file, directory_mesh, domain);
    }
#endif
    else if (type == DB_CURVE)
    {
        rv = GetCurve(domain_file, directory_mesh);
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    //
    // This may be leaked if an exception is thrown after it is allocated.
    // I'll live.
    //
    delete [] meshLocation;
    if (allocated_directory_mesh)
        delete [] directory_mesh;

    return rv;
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
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetUcdVar(DBfile *dbfile, const char *vname,
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
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray   *scalars = vtkFloatArray::New();

    //
    // Handle stripping out values for zone types we don't understand
    //
    bool arrayWasRemapped = false;
    if (uv->centering == DB_ZONECENT && metadata != NULL)
    {
        string meshName = metadata->MeshForVar(tvn);
        vector<int> zonesRangesToSkip = arbMeshZoneRangesToSkip[meshName];
        if (zonesRangesToSkip.size() > 0)
        {
            int numSkipped = ComputeNumZonesSkipped(zonesRangesToSkip);

            scalars->SetNumberOfTuples(uv->nels - numSkipped);
            float *ptr = (float *) scalars->GetVoidPointer(0);
            RemoveValuesForSkippedZones(zonesRangesToSkip,
                uv->vals[0], uv->nels, ptr);
            arrayWasRemapped = true;
        }
    }

    //
    // Populate the variable as we normally would.
    // This assumes it is a scalar variable.
    //
    if (arrayWasRemapped == false)
    {
        scalars->SetNumberOfTuples(uv->nels);
        float        *ptr     = (float *) scalars->GetVoidPointer(0);
        memcpy(ptr, uv->vals[0], sizeof(float)*uv->nels);
    }

    if (uv->mixvals != NULL && uv->mixvals[0] != NULL)
    {
        avtMixedVariable *mv = new avtMixedVariable(uv->mixvals[0], uv->mixlen,
                                                    tvn);
        void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
        cache->CacheVoidRef(tvn, AUXILIARY_DATA_MIXED_VARIABLE, timestep, 
                            domain, vr);
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

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkDataArray *scalars;
    if (qv->datatype == DB_DOUBLE)
        scalars = vtkDoubleArray::New();
    else
        scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(qv->nels);
    if (qv->major_order == DB_ROWMAJOR || qv->ndims <= 1)
    {
        int size = sizeof(float);
        if (qv->datatype == DB_DOUBLE)
            size = sizeof(double);
        void *ptr = scalars->GetVoidPointer(0);
        memcpy(ptr, qv->vals[0], size*qv->nels);
    }
    else
    {
        void *var2 = scalars->GetVoidPointer(0);
        void *var  = qv->vals[0];

        int nx = qv->dims[0];
        int ny = qv->dims[1];
        int nz = qv->ndims == 3 ? qv->dims[2] : 1;
        if (qv->datatype == DB_DOUBLE)
            CopyAndReorderQuadVar((double *) var2, nx, ny, nz, var);
        else
            CopyAndReorderQuadVar((float *) var2, nx, ny, nz, var);
    }

    if (qv->mixvals != NULL && qv->mixvals[0] != NULL)
    {
        avtMixedVariable *mv = new avtMixedVariable(qv->mixvals[0], qv->mixlen,
                                                    tvn);
        void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
        cache->CacheVoidRef(tvn, AUXILIARY_DATA_MIXED_VARIABLE, timestep, 
                            domain, vr);
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

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkFloatArray   *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(mv->nels);
    float        *ptr     = (float *) scalars->GetVoidPointer(0);
    memcpy(ptr, mv->vals[0], sizeof(float)*mv->nels);

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

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkFloatArray   *scalars = vtkFloatArray::New();
    scalars->SetNumberOfTuples(csgv->nels);
    float        *ptr     = (float *) scalars->GetVoidPointer(0);
    memcpy(ptr, csgv->vals[0], sizeof(float)*csgv->nels);

    DBFreeCsgvar(csgv);

    return scalars;
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
        EXCEPTION1(InvalidVariableException, meshname);
    }

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    vtkPoints            *points  = vtkPoints::New();
    points->SetNumberOfPoints(um->nnodes);
    float *pts = (float *) points->GetVoidPointer(0);
    int nnodes = um->nnodes;
    bool dim3 = (um->coords[2] != NULL ? true : false);
    float *tmp = pts;
    const float *coords0 = um->coords[0];
    const float *coords1 = um->coords[1];
    if (dim3)
    {
        const float *coords2 = um->coords[2];
        for (int i = 0 ; i < nnodes ; i++)
        {
            *tmp++ = *coords0++;
            *tmp++ = *coords1++;
            *tmp++ = *coords2++;
        }
    }
    else
    {
        for (int i = 0 ; i < nnodes ; i++)
        {
            *tmp++ = *coords0++;
            *tmp++ = *coords1++;
            *tmp++ = 0.;
        }
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
        //
        // Create a vtkInt array whose contents are the actual gnodeno data
        //
        vtkIntArray *arr = vtkIntArray::New();
        arr->SetNumberOfComponents(1);
        arr->SetNumberOfTuples(nnodes);
        int *ptr = arr->GetPointer(0);
        memcpy(ptr, um->gnodeno, nnodes*sizeof(int));

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
        vector<int> zoneRangesToSkip;
        ReadInConnectivity(ugrid, um->zones, um->zones->origin, zoneRangesToSkip);
        if (zoneRangesToSkip.size() > 0)
        {
            // squirl away knowledge of the zones we've removed
            arbMeshZoneRangesToSkip[mesh] = zoneRangesToSkip;
            int numSkipped = ComputeNumZonesSkipped(zoneRangesToSkip);

            // Issue a warning message about having skipped some zones
            char msg[1024];
            sprintf(msg, "\nIn reading mesh, \"%s\", VisIt encountered "
                "%d arbitrary polyhedral zones accounting for %3d %% "
                "of the zones in the mesh. Those zones have been removed. "
                "Future versions of VisIt will be able to display these "
                "zones. However, the current version cannot.", mesh,
                numSkipped, 100 * numSkipped / um->zones->nzones);
            avtCallback::IssueWarning(msg);
        }
        rv = ugrid;

        if (um->zones->gzoneno != NULL)
        {
            //
            // Create a vtkInt array whose contents are the actual gzoneno data
            //
            vtkIntArray *arr = vtkIntArray::New();
            arr->SetNumberOfComponents(1);
            arr->SetNumberOfTuples(um->zones->nzones);
            int *ptr = arr->GetPointer(0);
            memcpy(ptr, um->zones->gzoneno, um->zones->nzones*sizeof(int));

            //
            // Cache this VTK object but in the VoidRefCache, not the VTK cache
            // so that it can be obtained through the GetAuxiliaryData call
            //
            void_ref_ptr vr = void_ref_ptr(arr, avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef(mesh, AUXILIARY_DATA_GLOBAL_ZONE_IDS, timestep, 
                                domain, vr);
        }

    }
    else if (fl != NULL)
    {
        vtkPolyData *pd = vtkPolyData::New();
        fl->CalcFacelistFromPoints(points, pd);
        rv = pd;
    }

    points->Delete();
    DBFreeUcdmesh(um);

    return rv;
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
// ****************************************************************************

void
avtSiloFileFormat::ReadInConnectivity(vtkUnstructuredGrid *ugrid,
                                      DBzonelist *zl, int origin,
                                      vector<int> &zoneRangesToSkip)
{
    int   i, j, k;

    //
    // Tell the ugrid how many cells it will have.
    //
    int  numCells = 0;
    int  totalSize = 0;
    for (i = 0 ; i < zl->nshapes ; i++)
    {
        int vtk_zonetype = SiloZoneTypeToVTKZoneType(zl->shapetype[i]);
        if (vtk_zonetype != -2) // don't include arb. polyhedra
        {
            numCells += zl->shapecnt[i];
            totalSize += zl->shapecnt[i] * (zl->shapesize[i]+1);
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
    int *cl = cellLocations->GetPointer(0);

    int zoneIndex = 0;
    int currentIndex = 0;
    bool mustResize = false;
    int minIndexOffset = 0;
    int maxIndexOffset = 0;
    for (i = 0 ; i < zl->nshapes ; i++)
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
        // "Handle" arbitrary polyhedra by skipping over them
        //
        if (vtk_zonetype == -2) // DB_ZONETYPE_POLYHEDRON
        {
            zoneRangesToSkip.push_back(zoneIndex);
            zoneRangesToSkip.push_back(zoneIndex + shapecnt - 1);

            // keep track of adjustments we'll need to make to
            // min/max offsets for ghosting
            if (zoneIndex < zl->min_index)
            {
               if (zoneIndex + shapecnt < zl->min_index)
               {
                   minIndexOffset += shapecnt;
                   maxIndexOffset += shapecnt;
               }
               else
               {
                   minIndexOffset += (zl->min_index - zoneIndex); 
                   maxIndexOffset += (zl->min_index - zoneIndex); 
               }
            }
            else if (zoneIndex + shapecnt <= zl->max_index)
            {
               maxIndexOffset += shapecnt;
            }
            else if (zoneIndex + shapecnt > zl->max_index)
            {
               maxIndexOffset += (zl->max_index - zoneIndex + 1); 
            }

            nodelist += shapesize;
            zoneIndex += shapecnt;
        }
        else
        {
            bool tetsAreInverted = false;
            bool firstTet = true;
            for (j = 0 ; j < shapecnt ; j++)
            {
                *ct++ = effective_vtk_zonetype;
                *cl++ = currentIndex;

                if (vtk_zonetype != VTK_WEDGE &&
                    vtk_zonetype != VTK_PYRAMID &&
                    vtk_zonetype != VTK_TETRA &&
                    vtk_zonetype != -1)
                {
                    *nl++ = shapesize;
                    for (k = 0 ; k < shapesize ; k++)
                        *nl++ = *(nodelist+k) - origin;
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
                        tetsAreInverted = TetsAreInverted(nodelist, ugrid);
                        static bool haveIssuedWarning = false;
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

                nodelist += shapesize;
                currentIndex += effective_shapesize+1;
                zoneIndex++;
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
    //  Tell the ugrid which of its zones are real (avtGhostZone = 0),
    //  which are ghost (avtGhostZone = 1), but only create the ghost
    //  zones array if ghost zones are actually present.
    //
    const int first = zl->min_index - minIndexOffset;  // where the real zones start
    const int last = zl->max_index - maxIndexOffset;   // where the real zones end

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
        //  Give the array the proper name so that other vtk classes will
        //  recognize these as ghost levels.
        //
        vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
        ghostZones->SetName("avtGhostZones");
        ghostZones->SetNumberOfTuples(numCells);
        unsigned char *tmp = ghostZones->GetPointer(0);
        for (i = 0; i < first; i++)
        {
            //
            //  ghostZones at the begining of the zone list
            //
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val, 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            *tmp++ = val;
        }
        for (i = first; i <= last; i++)
        {
            //
            // real zones
            //
            *tmp++ = 0;
        }
        for (i = last+1; i < numCells; i++)
        {
            //
            //  ghostZones at the end of the zone list
            //
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val, 
                                          DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            *tmp++ = val;
        }
        ugrid->GetCellData()->AddArray(ghostZones);
        ghostZones->Delete();
        ugrid->SetUpdateGhostLevel(0);
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
//      mn       The domain.
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

    VerifyQuadmesh(qm, meshname);

    vtkDataSet *ds = NULL;
    if (qm->coordtype == DB_COLLINEAR)
    {
        ds = CreateRectilinearMesh(qm);
    }
    else
    {
        ds = CreateCurvilinearMesh(qm);
    }

    GetQuadGhostZones(qm, ds);

    //
    // Add group id as field data
    //
    vtkIntArray *grp_id_arr = vtkIntArray::New();
    grp_id_arr->SetNumberOfTuples(1);
    grp_id_arr->SetValue(0, qm->group_no);
    grp_id_arr->SetName("group_id");
    ds->GetFieldData()->AddArray(grp_id_arr);
    grp_id_arr->Delete();

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
        void_ref_ptr vr = cache->GetVoidRef("any_mesh",
                        AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1);
        if (*vr != NULL)
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
//
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetCurve(DBfile *dbfile, const char *cn)
{
    int i;

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

    //
    // Add all of the points to an array.
    //
    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();
    pd->SetPoints(pts);

    // DBForceSingle assures that all double data is converted to float
    // So, both are handled as float, here
    if (cur->datatype == DB_FLOAT ||
        cur->datatype == DB_DOUBLE)
    {
        vtkFloatArray *farr= vtkFloatArray::New();
        farr->SetNumberOfComponents(3);
        farr->SetNumberOfTuples(cur->npts);
        for (i = 0 ; i < cur->npts; i++)
            farr->SetTuple3(i, cur->x[i], cur->y[i], 0.0);
        pts->SetData(farr);
        farr->Delete();
    }
    else if (cur->datatype == DB_INT)
    {
        int *px = (int *) cur->x;
        int *py = (int *) cur->y;
        vtkIntArray *iarr= vtkIntArray::New();
        iarr->SetNumberOfComponents(3);
        iarr->SetNumberOfTuples(cur->npts);
        for (i = 0 ; i < cur->npts; i++)
            iarr->SetTuple3(i, px[i], py[i], 0);
        pts->SetData(iarr);
        iarr->Delete();
    }
    else if (cur->datatype == DB_SHORT)
    {
        short *px = (short *) cur->x;
        short *py = (short *) cur->y;
        vtkShortArray *sarr= vtkShortArray::New();
        sarr->SetNumberOfComponents(3);
        sarr->SetNumberOfTuples(cur->npts);
        for (i = 0 ; i < cur->npts; i++)
            sarr->SetTuple3(i, px[i], py[i], 0);
        pts->SetData(sarr);
        sarr->Delete();
    }
    else if (cur->datatype == DB_CHAR)
    {
        char *px = (char *) cur->x;
        char *py = (char *) cur->y;
        vtkCharArray *carr= vtkCharArray::New();
        carr->SetNumberOfComponents(3);
        carr->SetNumberOfTuples(cur->npts);
        for (i = 0 ; i < cur->npts; i++)
            carr->SetTuple3(i, px[i], py[i], 0);
        pts->SetData(carr);
        carr->Delete();
    }

    //
    // Connect the points up with line segments.
    //
    vtkCellArray *line = vtkCellArray::New();
    pd->SetLines(line);
    for (i = 1 ; i < cur->npts; i++)
    {
        line->InsertNextCell(2);
        line->InsertCellPoint(i-1);
        line->InsertCellPoint(i);
    }

    pts->Delete();
    line->Delete();

    DBFreeCurve(cur);

    return pd;
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
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::CreateRectilinearMesh(DBquadmesh *qm)
{
    int   i, j;

    vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    int           dims[3];
    vtkFloatArray   *coords[3];
    for (i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();

        if (i < qm->ndims)
        {
            dims[i] = qm->dims[i];
            coords[i]->SetNumberOfTuples(dims[i]);
            for (j = 0 ; j < dims[i] ; j++)
            {
                coords[i]->SetComponent(j, 0, qm->coords[i][j]);
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
// ****************************************************************************

template <class T>
static void CopyQuadCoordinates(T *dest, int nx, int ny, int nz, int morder,
    const T *const c0, const T *const c1, const T *const c2)
{
    int i, j, k;

    if (morder == DB_ROWMAJOR)
    {
        int nxy = nx * ny; 
        for (k = 0; k < nz; k++)
        {
            for (j = 0; j < ny; j++)
            {
                for (i = 0; i < nx; i++)
                {
                    int idx = k*nxy + j*nx + i;
                    *dest++ = c0 ? c0[idx] : 0.;
                    *dest++ = c1 ? c1[idx] : 0.;
                    *dest++ = c2 ? c2[idx] : 0.;
                }
            }
        }
    }
    else
    {
        int nyz = ny * nz; 
        for (k = 0; k < nz; k++)
        {
            for (j = 0; j < ny; j++)
            {
                for (i = 0; i < nx; i++)
                {
                    int idx = k + j*nz + i*nyz;
                    *dest++ = c0 ? c0[idx] : 0.;
                    *dest++ = c1 ? c1[idx] : 0.;
                    *dest++ = c2 ? c2[idx] : 0.;
                }
            }
        }
    }
}

vtkDataSet *
avtSiloFileFormat::CreateCurvilinearMesh(DBquadmesh *qm)
{
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
        CopyQuadCoordinates((double *) pts, nx, ny, nz, qm->major_order,
            (double *) qm->coords[0], (double *) qm->coords[1],
            qm->ndims <= 2 ? 0 : (double *) qm->coords[2]);
    }
    else
    {
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
        for (int ii = 0; ii < qm->nnodes; ii++)
            ghostPoints[ii] = true; 

        //
        // Set real values
        //
        for (int k = first[2]; k <= last[2]; k++)
            for (int j = first[1]; j <= last[1]; j++)
                for (int i = first[0]; i <= last[0]; i++)
                {
                    int index = k*dims[1]*dims[0] + j*dims[0] + i;
                    ghostPoints[index] = false; 
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

        ds->SetUpdateGhostLevel(0);
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
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetPointMesh(DBfile *dbfile, const char *mn)
{
    int   i, j;

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

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    vtkPoints *points  = vtkPoints::New();
    points->SetNumberOfPoints(pm->nels);
    float *pts = (float *) points->GetVoidPointer(0);
    for (i = 0 ; i < 3 ; i++)
    {
        float *tmp = pts + i;
        if (pm->coords[i] != NULL)
        {
            for (j = 0 ; j < pm->nels ; j++)
            {
                *tmp = pm->coords[i][j];
                tmp += 3;
            }
        }
        else
        {
            for (j = 0 ; j < pm->nels ; j++)
            {
                *tmp = 0.;
                tmp += 3;
            }
        }
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkUnstructuredGrid    *ugrid   = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints(points);
    ugrid->Allocate(pm->nels);
    vtkIdType onevertex[1];
    for (i = 0 ; i < pm->nels ; i++)
    {
        onevertex[0] = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
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
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetCSGMesh(DBfile *dbfile, const char *mn, int dom)
{
#ifndef DBCSG_INNER // remove after silo-4.5 is released
    return 0;
#else
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
    // Get the Silo construct.
    //
    DBcsgmesh *csgm = DBGetCsgmesh(dbfile, meshname);
    if (csgm == NULL)
    {
        EXCEPTION1(InvalidVariableException, meshname);
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

    return csggrid;

#endif
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
// ****************************************************************************

void
avtSiloFileFormat::DetermineFilenameAndDirectory(char *input,
    const char *mdirname, char *filename, char *&location,
    bool *allocated_location)
{
    if (allocated_location)
        *allocated_location = false;

    //
    // Figure out if there is a ':' in the string.
    //
    char *p = strstr(input, ":");

    if (p == NULL)
    {
        //
        // There is no colon, so the variable must be in the current file.
        // Leave the file handle alone.
        //
        strcpy(filename, ".");
	if (mdirname == 0 || strcmp(input, "EMPTY") == 0 || input[0] == '/' ||
	   (input[0] == '.' && input[1] == '/'))
        {
           location = input;
	}
        else
	{
	    if (!strncmp(mdirname, input, strlen(mdirname)) == 0)
	    {
	        char tmp[1024];
	        sprintf(tmp, "%s/%s", mdirname, input);
	        location = CXX_strdup(tmp);
	        if (allocated_location)
	            *allocated_location = true;
	    }
	    else
	    {
	        location = input;
	    }
	}
    }
    else
    {
        //
        // Make a copy of the string all the way up to the colon.
        //
        strncpy(filename, input, p-input);
        filename[p-input] = '\0';

        //
        // The location of the variable is *one after* the colon.
        //
        location = p+1;
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
// ****************************************************************************

void
avtSiloFileFormat::DetermineFileAndDirectory(char *input, DBfile *&cFile,
    const char *meshDirname, char *&location, bool *allocated_location)
{
    char filename[1024];
    char cwd[1024];
    DetermineFilenameAndDirectory(input, meshDirname, filename, location,
        allocated_location);
    if (strcmp(filename, ".") != 0)
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
        cFile = OpenFile(filename, skipGlobalInfo);
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
// ****************************************************************************

string
avtSiloFileFormat::DetermineMultiMeshForSubVariable(DBfile *dbfile,
                                                    const char *name,
                                                    char **varname,
                                                    int nblocks,
                                                    const char *curdir)
{
    int i;
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
    while (string(varname[meshnum]) == "EMPTY")
    {
        meshnum++;
        if (meshnum >= nblocks)
        {
            EXCEPTION1(InvalidVariableException,  name);
        }
    }

    GetMeshname(dbfile, varname[meshnum], subMesh);

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
    GetRelativeVarName(varname[meshnum], subMesh, subMeshWithFile);
    if (subMesh[0] == '/')
        GetRelativeVarName(varname[meshnum], subMeshTmp, subMeshWithFileTmp);

    //
    // Attempt an "exact" match, where the first mesh for the multivar is
    // an exact match and the number of domains is the same.
    //
    int size = actualMeshName.size();
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
    for (i = 0 ; i < size ; i++)
    {
        if (firstSubMeshVarName[i] == varmesh &&
            blocksForMesh[i] == nblocks)
        {
#ifndef MDSERVER

            string *dirs = new string[nblocks];
            for (int k = 0; k < nblocks; k++)
                SplitDirVarName(varname[k], curdir, dirs[k], varmesh);

            for (int j = 0; j < allSubMeshDirs[i].size(); j++)
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
            name, varname[meshnum], subMeshWithFile);
    EXCEPTION1(SiloException, str);
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMeshtype
//
//  Purpose:
//      Gets the mesh type for a variable, even if it is in a different file.
//
//  Arguments:
//      dbfile    The dbfile that mesh came from.
//      mesh      A mesh name, possibly with a prepended directory and filename
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2001
//
//  Modifications:
//
//      Sean Ahern, Fri Feb  8 13:57:12 PST 2002
//      Added error checking.
//
// ****************************************************************************

int
avtSiloFileFormat::GetMeshtype(DBfile *dbfile, char *mesh)
{
    char   *dirvar;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(mesh, correctFile, 0, dirvar);
    int rv = DBInqMeshtype(correctFile, dirvar);
    if (rv < 0)
    {
        char str[1024];
        sprintf(str, "Unable to determine mesh type for \"%s\".", mesh);
        EXCEPTION1(SiloException, str);
    }
    return rv;
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
// ****************************************************************************

void
avtSiloFileFormat::GetMeshname(DBfile *dbfile, char *var, char *meshname)
{
    char   *dirvar;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(var, correctFile, 0, dirvar);
    int rv = DBInqMeshname(correctFile, dirvar, meshname);
    if (rv < 0)
    {
        char str[1024];
        sprintf(str, "Unable to determine mesh for %s.", var);
        EXCEPTION1(SiloException, str);
    }
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
// ****************************************************************************

void *
avtSiloFileFormat::GetComponent(DBfile *dbfile, char *var, char *compname)
{
    char   *dirvar;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(var, correctFile, 0, dirvar);
    void *rv = DBGetComponent(correctFile, dirvar, compname);
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
// ****************************************************************************

void *
avtSiloFileFormat::GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df)
{
    // for CSG meshes, each domain is a csgregion and a group of regions
    // forms a visit "domain". So, we need to re-map the domain id
    metadata->ConvertCSGDomainToBlockAndRegion(var, &domain, 0);

    void *rv = NULL;
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
#if DISABLE_EXTENTS_DUE_TO_BAD_ALE3D_FILES!=1
#foo // cause compiler to fail if we get here
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        rv = (void *) GetSpatialExtents(var);
        df = avtIntervalTree::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0)
    {
        rv = (void *) GetDataExtents(var);
        df = avtIntervalTree::Destruct;
    }
#endif

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
    DBmultimat *mm = QueryMultimat("", m);
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

    char        *matname = NULL;

    if (type == DB_MULTIMAT)
    {
        if (mm == NULL)
            mm = GetMultimat("", m);
        if (mm == NULL)
            EXCEPTION1(InvalidVariableException, m);
        if (dom >= mm->nmats || dom < 0 )
        {
            EXCEPTION2(BadDomainException, dom, mm->nmats);
        }

        if (strcmp(mm->matnames[dom], "EMPTY") == 0)
            return NULL;

        matname = CXX_strdup(mm->matnames[dom]);
    }
    else // (type == DB_MATERIAL)
    {
        if (dom != 0)
        {
            EXCEPTION2(BadDomainException, dom, 1);
        }
        matname = CXX_strdup(mat);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    char   *directory_mat = NULL;
    DetermineFileAndDirectory(matname, domain_file, 0, directory_mat);

    avtMaterial *rv = CalcMaterial(domain_file, directory_mat, mat, dom);

    if (matname != NULL)
    {
        delete [] matname;
        matname = NULL;
    }

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
    DBmultimatspecies *mm = QueryMultimatspec("", s);
    int type;
    if (mm != NULL)
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

    DBmultimatspecies  *ms = NULL;
    char               *specname = NULL;

    if (type == DB_MULTIMATSPECIES)
    {
        if (ms == NULL)
            ms = GetMultimatspec("", s);
        if (ms == NULL)
            EXCEPTION1(InvalidVariableException, s);
        if (dom >= ms->nspec || dom < 0 )
        {
            EXCEPTION2(BadDomainException, dom, ms->nspec);
        }

        if (strcmp(ms->specnames[dom], "EMPTY") == 0)
            return NULL;

        specname = CXX_strdup(ms->specnames[dom]);
    }
    else // (type == DB_MATSPECIES)
    {
        if (dom != 0)
        {
            EXCEPTION2(BadDomainException, dom, 1);
        }
        specname = CXX_strdup(spec);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    char   *directory_spec = NULL;
    DetermineFileAndDirectory(specname, domain_file, 0, directory_spec);

    avtSpecies *rv = CalcSpecies(domain_file, directory_spec);

    if (specname != NULL)
    {
        delete [] specname;
        specname = NULL;
    }

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
    int type = DBInqVarType(dbfile, m);

    if (type != DB_MULTIMESH && type != DB_UCDMESH)
    {
        //
        // This is not a ucd mesh or a multi-mesh, so it does not have a
        // facelist.
        //
        return NULL;
    }

    DBmultimesh *mm = NULL;
    char        *meshname = NULL;

    if (type == DB_MULTIMESH)
    {
        mm = GetMultimesh("", m);
        if (mm == NULL)
            EXCEPTION1(InvalidFilesException, m);
        if (dom >= mm->nblocks || dom < 0 )
        {
            EXCEPTION2(BadDomainException, dom, mm->nblocks);
        }
        if (mm->meshtypes[dom] != DB_UCDMESH)
        {
            return NULL;
        }
        meshname = CXX_strdup(mm->meshnames[dom]);
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
    char   *directory_mesh = NULL;
    DetermineFileAndDirectory(meshname, domain_file, 0, directory_mesh);

    avtFacelist *rv = CalcExternalFacelist(domain_file, directory_mesh);

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
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetGlobalNodeIds(int dom, const char *mesh)
{
    debug5 << "Reading in domain " << dom << ", global node ids for " << mesh << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

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
    char   *directory_mesh = NULL;
    DetermineFileAndDirectory(meshname, domain_file, 0, directory_mesh);

    // We want to get just the global node ids.  So we need to get the ReadMask,
    // set it to read global node ids, then set it back.
    long mask = DBGetDataReadMask();
    DBSetDataReadMask(DBUMGlobNodeNo);
    DBucdmesh *um = DBGetUcdmesh(domain_file, directory_mesh);
    DBSetDataReadMask(mask);
    if (um == NULL)
        EXCEPTION1(InvalidVariableException, mesh);

    vtkIntArray *rv = NULL;
    if (um->gnodeno != NULL)
    {
        //
        // Create a vtkInt array whose contents are the actual gnodeno data
        //
        rv = vtkIntArray::New();
        rv->SetArray(um->gnodeno, um->nnodes, 0);
        rv->SetNumberOfComponents(1);

        //
        // Since vtkIntArray now owns the data, we remove it from um
        //
        um->gnodeno = NULL;
    }

    DBFreeUcdmesh(um);

    delete [] meshname;

    return rv;
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
    char   *directory_mesh = NULL;
    DetermineFileAndDirectory(meshname, domain_file, 0, directory_mesh);

    // We want to get just the global node ids.  So we need to get the ReadMask,
    // set it to read global node ids, then set it back.
    long mask = DBGetDataReadMask();
    DBSetDataReadMask(DBUMZonelist|DBZonelistGlobZoneNo);
    DBucdmesh *um = DBGetUcdmesh(domain_file, directory_mesh);
    DBSetDataReadMask(mask);
    if (um == NULL)
        EXCEPTION1(InvalidVariableException, mesh);

    vtkIntArray *rv = NULL;
    if (um->zones->gzoneno != NULL)
    {
        //
        // Create a vtkInt array whose contents are the actual gnodeno data
        //
        rv = vtkIntArray::New();
        rv->SetArray(um->zones->gzoneno, um->zones->nzones, 0);
        rv->SetNumberOfComponents(1);

        //
        // Since vtkIntArray now owns the data, we remove it from um
        //
        um->zones->gzoneno = NULL;
    }

    DBFreeUcdmesh(um);

    delete [] meshname;

    return rv;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetSpatialExtents
//
//  Purpose:
//      Gets the sptial extents from a multimesh, if they exist in the file 
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
    DBmultimesh *mm = QueryMultimesh("", mesh);
    if (mm == NULL)
        mm = GetMultimesh("", mesh);

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
    DBmultivar *mv = QueryMultivar("", var);
    if (mv == NULL)
        mv = GetMultivar("", var);

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
// ****************************************************************************

avtMaterial *
avtSiloFileFormat::CalcMaterial(DBfile *dbfile, char *matname, const char *tmn,
    int dom)
{
    DBmaterial *silomat = DBGetMaterial(dbfile, matname);
    if (silomat == NULL)
    {
        EXCEPTION1(InvalidVariableException, matname);
    }

    char dom_string[128];
    sprintf(dom_string, "Domain %d", dom);

    //
    // If we have material names, create a new set of material names that have
    // the material number encoded.
    //
    char **matnames = NULL;
    char *buffer = NULL;
    if (silomat->matnames != NULL)
    {
        int nmat = silomat->nmat;
        
        
        int max_dlen = 0;
        for (int i = 0 ; i < nmat ; i++)
        {
            int dlen =int(log10(float(silomat->matnos[i]+1))) + 1;
            if(dlen>max_dlen)
                max_dlen = dlen;
        }
        
        matnames = new char*[nmat];
        buffer = new char[nmat*256 + max_dlen];
        
        for (int i = 0 ; i < nmat ; i++)
        {
            matnames[i] = buffer + (256+max_dlen)*i;
            sprintf(matnames[i], "%d %s", silomat->matnos[i],
                                          silomat->matnames[i]);
        }
    }

    //
    // Handle cases were zones may have been removed due to unknown zone type
    // Note: We can get away with remapping only the matlist array 
    // Note: We should only wind up doing this for an unstructured mesh
    //
    int *matList = silomat->matlist;

    // handle cases where we really have a 1d matlist array even when ndims
    // is greater than 1
    int numDimsNonUnity = 0;
    int nonUnityDim = -1;
    int ndims = silomat->ndims;
    int dims[3];
    for (int i = 0; i < silomat->ndims; i++)
    {
        if (silomat->dims[i] != 1)
        {
            numDimsNonUnity++;
            nonUnityDim = i;
        }
        dims[i] = silomat->dims[i];
    }

    if (metadata != NULL && numDimsNonUnity == 1)
    {
        string meshName = metadata->MeshForVar(tmn);
        vector<int> zonesRangesToSkip = arbMeshZoneRangesToSkip[meshName];
        if (zonesRangesToSkip.size() > 0)
        {
            int numSkipped = ComputeNumZonesSkipped(zonesRangesToSkip);
            matList = new int[dims[nonUnityDim] - numSkipped];
            RemoveValuesForSkippedZones(zonesRangesToSkip,
                silomat->matlist, dims[nonUnityDim], matList);
            dims[nonUnityDim] -= numSkipped;
        }
    }

    avtMaterial *mat = new avtMaterial(silomat->nmat,
                                       silomat->matnos,
                                       matnames,
                                       ndims,
                                       dims,
                                       silomat->major_order,
                                       matList,
                                       silomat->mixlen,
                                       silomat->mix_mat,
                                       silomat->mix_next,
                                       silomat->mix_zone,
                                       silomat->mix_vf,
                                       dom_string
#ifdef DBOPT_ALLOWMAT0
                                       ,silomat->allowmat0
#endif
                                       );

    if (matList != silomat->matlist)
        delete [] matList;
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
// ****************************************************************************

avtSpecies *
avtSiloFileFormat::CalcSpecies(DBfile *dbfile, char *specname)
{
    DBmatspecies *silospec = DBGetMatspecies(dbfile, specname);
    if (silospec == NULL)
    {
        EXCEPTION1(InvalidVariableException, specname);
    }

    avtSpecies *spec = new avtSpecies(silospec->nmat,
                                      silospec->nmatspec,
                                      silospec->ndims,
                                      silospec->dims,
                                      silospec->speclist,
                                      silospec->mixlen,
                                      silospec->mix_speclist,
                                      silospec->nspecies_mf,
                                      silospec->species_mf);

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
// ****************************************************************************

avtFacelist *
avtSiloFileFormat::CalcExternalFacelist(DBfile *dbfile, char *mesh)
{
    char   *realvar = NULL;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(mesh, correctFile, 0, realvar);

    // We want to get just the facelist.  So we need to get the ReadMask,
    // set it to read facelists, then set it back.
    long mask = DBGetDataReadMask();
    DBSetDataReadMask(DBUMFacelist | DBFacelistInfo);
    DBucdmesh *um = DBGetUcdmesh(correctFile, realvar);
    DBSetDataReadMask(mask);
    if (um == NULL)
        EXCEPTION1(InvalidVariableException, mesh);
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
//  Method: avtSiloFileFormat::PopulateIOInformation
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
// ****************************************************************************

void
avtSiloFileFormat::PopulateIOInformation(avtIOInformation &ioInfo)
{
    int   i, j;
    int nMeshes = metadata->GetNumMeshes();

    if (nMeshes < 1)
    {
        debug1 << "Cannot populate I/O info since there are no meshes" << endl;
        ioInfo.SetNDomains(0);
        return;
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
            return;
        }
    }

    if (blocks <= 1)
    {
        debug5 << "No need to do I/O optimization because there is only "
               << "one block" << endl;
        ioInfo.SetNDomains(1);
        return;
    }

    //
    // Looks like all of the meshes are the same size.  ASSUME they are all
    // the same and pick the first one.  This is done for I/O optimizations, so
    // it is okay to be wrong if our assumption is not true.
    //
    string meshname = metadata->GetMesh(firstNonCSGMesh)->name;

    DBmultimesh *mm = GetMultimesh("", meshname.c_str());
    if (mm == NULL)
        EXCEPTION1(InvalidFilesException, meshname.c_str());

    vector<string> filenames;
    vector<vector<int> > groups;
    for (i = 0 ; i < mm->nblocks ; i++)
    {
        char filename[1024];
        char *location = NULL;
        DetermineFilenameAndDirectory(mm->meshnames[i], 0, filename, location);
        int index = -1;
        for (j = 0 ; j < filenames.size() ; j++)
        {
            if (filename == filenames[j])
            {
                index = j;
                break;
            }
        }
        if (index == -1)
        {
            filenames.push_back(string(filename));
            vector<int> newvector_placeholder;
            groups.push_back(newvector_placeholder);
            index = filenames.size()-1;
        }
        groups[index].push_back(i);
    }

    ioInfo.SetNDomains(mm->nblocks);
    ioInfo.AddHints(groups);
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
// ****************************************************************************

bool
avtSiloFileFormat::ShouldGoToDir(const char *dirname)
{
    if (domainDirs.count(dirname) == 0)
        debug5 << "Deciding to go into dir \"" << dirname << "\"" << endl;
    else
        debug5 << "Skipping dir \"" << dirname << "\"" << endl;
    return (domainDirs.count(dirname) == 0);
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
// ****************************************************************************

void
avtSiloFileFormat::RegisterDomainDirs(const char * const *dirlist, int nList,
                                      const char *curDir)
{
    for (int i = 0 ; i < nList ; i++)
    {
        if (strcmp(dirlist[i], "EMPTY") == 0)
            continue;

        string str = PrepareDirName(dirlist[i], curDir);
        domainDirs.insert(str);
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultimesh
//
//  Purpose:
//      Returns a multimesh from the cache.  Only returns a multimesh if
//      we have called "GetMultimesh" on it before.  It is entirely possible
//      that "QueryMultimesh" returns NULL and there *is* a multi-mesh.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
// ****************************************************************************

DBmultimesh *
avtSiloFileFormat::QueryMultimesh(const char *path, const char *name)
{
    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);
       
    //
    // First, check to see if we have already gotten the multi-mesh.
    //
    for (int i = 0 ; i < multimeshes.size() ; i++)
        if (multimesh_name[i] == combined_name)
            return multimeshes[i];

    return NULL;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultimesh
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
// ****************************************************************************

DBmultimesh *
avtSiloFileFormat::GetMultimesh(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-mesh.
    //
    DBmultimesh *qm = QueryMultimesh(path, name);
    if (qm != NULL)
        return qm;

    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    //
    // We haven't seen this multimesh before -- read it in.
    //
    DBfile *dbfile = GetFile(tocIndex);
    DBmultimesh *mm = DBGetMultimesh(dbfile, combined_name);

    multimesh_name.push_back(combined_name);
    multimeshes.push_back(mm);

    return mm;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultivar
//
//  Purpose:
//      Returns a multivar from the cache.  Only returns a multivar if
//      we have called "GetMultivar" on it before.  It is entirely possible
//      that "QueryMultivar" returns NULL and there *is* a multi-var.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
// ****************************************************************************

DBmultivar *
avtSiloFileFormat::QueryMultivar(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-var.
    //
    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    for (int i = 0 ; i < multivars.size() ; i++)
        if (multivar_name[i] == combined_name)
            return multivars[i];

    return NULL;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultivar
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
// ****************************************************************************

DBmultivar *
avtSiloFileFormat::GetMultivar(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-var.
    //
    DBmultivar *qm = QueryMultivar(path, name);
    if (qm != NULL)
        return qm;

    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    //
    // We haven't seen this multivar before -- read it in.
    //
    DBfile *dbfile = GetFile(tocIndex);
    DBmultivar *mm = DBGetMultivar(dbfile, combined_name);

    multivar_name.push_back(combined_name);
    multivars.push_back(mm);

    return mm;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultimat
//
//  Purpose:
//      Returns a multimat from the cache.  Only returns a multimat if
//      we have called "GetMultimat" on it before.  It is entirely possible
//      that "QueryMultimat" returns NULL and there *is* a multi-mat.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
// ****************************************************************************

DBmultimat *
avtSiloFileFormat::QueryMultimat(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-mat.
    //
    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    for (int i = 0 ; i < multimats.size() ; i++)
        if (multimat_name[i] == combined_name)
            return multimats[i];

    return NULL;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultimat
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
// ****************************************************************************

DBmultimat *
avtSiloFileFormat::GetMultimat(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-mat.
    //
    DBmultimat *qm = QueryMultimat(path, name);
    if (qm != NULL)
        return qm;

    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    //
    // We haven't seen this multimat before -- read it in.
    //
    DBfile *dbfile = GetFile(tocIndex);
    DBmultimat *mm = DBGetMultimat(dbfile, combined_name);

    multimat_name.push_back(combined_name);
    multimats.push_back(mm);

    return mm;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultimatspec
//
//  Purpose:
//      Returns a multimatspec from the cache.  Only returns a multimatspec if
//      we have called "GetMultimatspec" on it before.  It is entirely possible
//      that "QueryMultimatspec" returns NULL and there *is* a multi-matspec.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
// ****************************************************************************

DBmultimatspecies *
avtSiloFileFormat::QueryMultimatspec(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-matspec.
    //
    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    for (int i = 0 ; i < multimatspecies.size() ; i++)
        if (multimatspec_name[i] == combined_name)
            return multimatspecies[i];

    return NULL;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultimatspec
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
// ****************************************************************************

DBmultimatspecies *
avtSiloFileFormat::GetMultimatspec(const char *path, const char *name)
{
    //
    // First, check to see if we have already gotten the multi-matspec.
    //
    DBmultimatspecies *qm = QueryMultimatspec(path, name);
    if (qm != NULL)
        return qm;

    char combined_name[1024];
    if ((path == NULL) || (strcmp(path, "") == 0) || (strcmp(path, "/") == 0))
        strcpy(combined_name, name);
    else
        sprintf(combined_name, "%s/%s", path, name);

    //
    // We haven't seen this multimatspec before -- read it in.
    //
    DBfile *dbfile = GetFile(tocIndex);
    DBmultimatspecies *mm = DBGetMultimatspecies(dbfile, combined_name);

    multimatspec_name.push_back(combined_name);
    multimatspecies.push_back(mm);

    return mm;
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
// ****************************************************************************
void
ExceptionGenerator(char *msg)
{
    if (msg)
    {
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
// ****************************************************************************

char *
GenerateName(const char *dirname, const char *varname)
{
    if (varname[0] == '/')
    {
        int len = strlen(varname);
        int num_slash = 0;
        for (int i = 0 ; i < len ; i++)
            if (varname[i] == '/')
                num_slash++;

        //
        // If there are lots of slashes, then we have a fully qualified path,
        // so leave them all in.  If there is only one slash (and it is the
        // first one), then take out the slash -- since the var would be
        // referred to as "Mesh", not "/Mesh".
        //
        int offset = (num_slash > 1 ? 0 : 1); 
        char *rv = new char[strlen(varname)+1];
        strcpy(rv, varname+offset);
        return rv;
    }

    int amtForSlash = 1;
    int amtForNullChar = 1;
    int amtForMiddleSlash = 1;
    int len = strlen(dirname) + strlen(varname) - amtForSlash + amtForNullChar
                                                + amtForMiddleSlash;

    char *rv = new char[len];

    const char *dir_without_leading_slash = dirname+1;

    bool needMiddleSlash = false;
    if (strlen(dir_without_leading_slash) > 0)
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
    int dirlen = 0;
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
    int len;

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
    int dirlen = 0;
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
//  Function: TetsAreInverted 
//
//  Purpose: Determine if Tets in Silo are inverted from Silo's Normal ordering
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 21, 2007 
//
// ****************************************************************************

bool
TetsAreInverted(const int *siloTetrahedron, vtkUnstructuredGrid *ugrid)
{
    //
    // initialize set of 4 points of tet
    //
    float *pts = (float *) ugrid->GetPoints()->GetVoidPointer(0); 
    float p[4][3];
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
            p[i][j] = pts[3*siloTetrahedron[i] + j];
    }

    //
    // Compute a vector normal to plane of first 3 points
    //
    float n1[3] = {p[1][0] - p[0][0], p[1][1] - p[0][1], p[1][2] - p[0][2]};
    float n2[3] = {p[2][0] - p[0][0], p[2][1] - p[0][1], p[2][2] - p[0][2]};
    float n1Xn2[3] = {  n1[1]*n2[2] - n1[2]*n2[1],
                      -(n1[0]*n2[2] - n1[2]*n2[0]),
                        n1[0]*n2[1] - n1[1]*n2[0]};
    
    //
    // Compute a dot-product of normal with a vector to the 4th point.
    // If the tet is specified as Silo normally expects it, this dot
    // product should be negative. If it is not negative, then tets
    // are inverted
    //
    float n3[3] = {p[3][0] - p[0][0], p[3][1] - p[0][1], p[3][2] - p[0][2]};
    float n3Dotn1Xn2 = n3[0]*n1Xn2[0] + n3[1]*n1Xn2[1] + n3[2]*n1Xn2[2];

    if (n3Dotn1Xn2 > 0)
        return true;
    else
        return false;
}


// ****************************************************************************
//  Function: ComputeNumZonesSkipped 
//
//  Purpose: Determine total number of zones represented in a set of skip
//  ranges.
//
//  Programmer: Mark C. Miller 
//  Creation:   October 21, 2004 
//
// ****************************************************************************

int
ComputeNumZonesSkipped(vector<int>& zoneRangesSkipped)
{
   int retVal = 0;
   for (int i = 0; i < zoneRangesSkipped.size(); i+=2)
       retVal += (zoneRangesSkipped[i+1] - zoneRangesSkipped[i] + 1);
   return retVal;
}

// ****************************************************************************
//  Function: RemoveValuesForSkippedZones
//
//  Purpose: Given an input and output array, remove values from the input
//  array that are for zones that are in the skip ranges.
//
//  Programmer: Mark C. Miller 
//  Creation:   October 21, 2004 
//
// ****************************************************************************
template<class T>
static void RemoveValuesForSkippedZones(vector<int>& zoneRangesSkipped,
                T *inArray, int inArraySize, T *outArray)
{
    int skipRangeIndexToUse = 0;
    int inArrayIndex = 0;
    int outArrayIndex = 0;

    while (inArrayIndex < inArraySize)
    {
        while (inArrayIndex == zoneRangesSkipped[skipRangeIndexToUse])
        {
            inArrayIndex += (zoneRangesSkipped[skipRangeIndexToUse+1] -
                             zoneRangesSkipped[skipRangeIndexToUse] + 1);
            skipRangeIndexToUse += 2;
        }

        outArray[outArrayIndex] = inArray[inArrayIndex];

        outArrayIndex++;
        inArrayIndex++;
    }
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
        int i;
        string tmpStr;

        char  *vars = new char[lvars+1];
        for (i = 0 ; i < lvars+1 ; i++)
            vars[i] = '\0';
        DBReadVar(dbfile, "MultivarToMultimeshMap_vars", vars);

        vector<string> varVec;
        tmpStr.clear();
        for (i = 0 ; i < lvars+1; i++)
        {
            if (vars[i] == ';' || (i == lvars))
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
        for (i = 0 ; i < lmeshes+1 ; i++)
            meshes[i] = '\0';
        DBReadVar(dbfile, "MultivarToMultimeshMap_meshes", meshes);

        vector<string> meshVec;
        tmpStr.clear();
        for (i = 0 ; i < lmeshes+1; i++)
        {
            if (meshes[i] == ';' || (i == lmeshes))
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
            EXCEPTION1(InvalidVariableException, "MultivarToMultimeshMap: "
                "vars|meshes have different number of entries");
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
// ****************************************************************************
static string
GuessCodeNameFromTopLevelVars(DBfile *dbfile)
{
    string retval;

    if (DBInqVarExists(dbfile, "lineage") &&
        DBInqVarExists(dbfile, "version_number") &&
        DBInqVarExists(dbfile, "znburn_flag") &&
        DBInqVarExists(dbfile, "chemistry_flag"))
        retval = "Ale3d";
    else
        retval = "Unknown";

    debug5 << "Looks like this Silo file was produced by code \"" << retval << "\"" << endl;
    return retval;
}

// ****************************************************************************
//  Function: AddAle3drlxstatEnumerationInfo
//
//  Purpose: Add enumeration info for Ale3d's rlxstat variable. These names and
//  values were taken directly from Ale3d's RelaxTest.h file.
//  Programmer: Mark C. Miller 
//  Creation:   July 9, 2007 
//
// ****************************************************************************
static void
AddAle3drlxstatEnumerationInfo(avtScalarMetaData *smd)
{
    smd->isEnumeration = true;
    smd->enumNames.push_back("RLX_Uninitialized_-1");
    smd->enumNames.push_back("RLX_Constrained_0");
    smd->enumNames.push_back("RLX_JustRelaxed_1");
    smd->enumNames.push_back("RLX_Relaxing_2");
    smd->enumNames.push_back("RLX_MustRelax_3");
    smd->enumNames.push_back("RLX_DispShortEdge_4");
    smd->enumNames.push_back("RLX_DispLagMotion_5");
    smd->enumNames.push_back("RLX_InflowOutflow_6");
    smd->enumNames.push_back("RLX_Symmetry_7");
    smd->enumNames.push_back("RLX_AngleWall_8");
    smd->enumNames.push_back("RLX_MustRelaxLimited_9");
    smd->enumNames.push_back("RLX_AdvectTangential_10");
    smd->enumNames.push_back("RLX_Reaction_13");
    smd->enumNames.push_back("RLX_ChemGrad_14");
    smd->enumNames.push_back("RLX_HoldNodeset_15");
    smd->enumNames.push_back("RLX_IntHist_16");
    smd->enumNames.push_back("RLX_Velocity_20");
    smd->enumNames.push_back("RLX_LightingTime_21");
    smd->enumNames.push_back("RLX_Region_22");
    smd->enumNames.push_back("RLX_ZNBurn_23");
    smd->enumNames.push_back("RLX_PlasticStrain_24");
    smd->enumNames.push_back("RLX_SALE_25");
    smd->enumNames.push_back("RLX_AdvTime_26");
    smd->enumNames.push_back("RLX_HoldUntilActive_27");
    smd->enumNames.push_back("RLX_SlaveExtension_29");
    smd->enumNames.push_back("RLX_PartialInvalid_30");
    smd->enumNames.push_back("RLX_FreeSurface_31");
    smd->enumNames.push_back("RLX_MixedNode_32");
    smd->enumNames.push_back("RLX_IgnitionPt_33");
    smd->enumNames.push_back("RLX_SlideMaster_34");
    smd->enumNames.push_back("RLX_PeriodicRelax_35");
    smd->enumNames.push_back("RLX_HoldUntilGrace_36");
    smd->enumNames.push_back("RLX_HeldIntHist_37");
    smd->enumNames.push_back("RLX_Equilibrated_38");
    smd->enumNames.push_back("RLX_ShellNode_39");
    smd->enumNames.push_back("RLX_BeamNode_40");
    smd->enumValues.push_back(-1);
    smd->enumValues.push_back(0);
    smd->enumValues.push_back(1);
    smd->enumValues.push_back(2);
    smd->enumValues.push_back(3);
    smd->enumValues.push_back(4);
    smd->enumValues.push_back(5);
    smd->enumValues.push_back(6);
    smd->enumValues.push_back(7);
    smd->enumValues.push_back(8);
    smd->enumValues.push_back(9);
    smd->enumValues.push_back(10);
    smd->enumValues.push_back(13);
    smd->enumValues.push_back(14);
    smd->enumValues.push_back(15);
    smd->enumValues.push_back(16);
    smd->enumValues.push_back(20);
    smd->enumValues.push_back(21);
    smd->enumValues.push_back(22);
    smd->enumValues.push_back(23);
    smd->enumValues.push_back(24);
    smd->enumValues.push_back(25);
    smd->enumValues.push_back(26);
    smd->enumValues.push_back(27);
    smd->enumValues.push_back(29);
    smd->enumValues.push_back(30);
    smd->enumValues.push_back(31);
    smd->enumValues.push_back(32);
    smd->enumValues.push_back(33);
    smd->enumValues.push_back(34);
    smd->enumValues.push_back(35);
    smd->enumValues.push_back(36);
    smd->enumValues.push_back(37);
    smd->enumValues.push_back(38);
    smd->enumValues.push_back(39);
    smd->enumValues.push_back(40);
}
