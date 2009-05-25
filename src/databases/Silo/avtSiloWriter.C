/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              avtSiloWriter.C                              //
// ************************************************************************* //

#include <avtSiloWriter.h>
#include <avtSiloOptions.h>

#include <float.h>
#include <ctype.h>

#include <vector>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>

#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <Utility.h>

#include <silo.h>


using std::map;
using std::string;
using std::vector;
using namespace SiloDBOptions;

// ****************************************************************************
//  Function: VN (Valid Silo Variable Name) 
//
//  Purpose: Ensure a given object name being passed to Silo is a valid name
//      for a silo object. The function is desgined to be used 'in-place' in
//      a Silo function call where it may be used for multiple args in the 
///     call. 
//
//  Programmer: Mark C. Miller, Tue Aug 26 14:18:13 PDT 2008
//
//  Modifications:
//    Brad Whitlock, Fri Mar 6 09:56:33 PDT 2009
//    Allow / since we'll get rid of them using another method where we use
//    them to create directories.
//
// ****************************************************************************
static const char *VN(const string &n)
{
    static int k = 0;
    static string vn[10];
    int km = k % 10;

    k++;
    vn[km] = n;
    for (size_t i = 0; i < vn[km].size(); i++)
    {
        if (isalnum(vn[km][i]) || vn[km][i] == '_' || vn[km][i] == '/')
            continue;
        vn[km][i] = '_';
    }

    return vn[km].c_str();
}

// ****************************************************************************
// Function: BeginVar
//
// Purpose: 
//   Splits a name containing '/' into separate names and makes Silo directories
//   for the names up to the last name, which is the name of an object.
//
// Arguments:
//   dbfile  : The Silo file.
//   name    : The name to be split.
//   nlevels : The number of levels in the name.
//
// Returns:    The object name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 6 15:31:08 PST 2009
//
// Modifications:
//   
// ****************************************************************************

static string 
BeginVar(DBfile *dbfile, const string &name, int &nlevels)
{
    stringVector names = SplitValues(name, '/');
    nlevels = 0;
    for(size_t i = 0; i < names.size()-1; ++i)
    {
        int t = DBInqVarType(dbfile, names[i].c_str());
        if(t != DB_DIR)
            DBMkDir(dbfile, names[i].c_str());
        DBSetDir(dbfile, names[i].c_str());
        ++nlevels;
    }
    string s(name);
    if(names.size() > 0)
        s = names[names.size()-1];
    VN(s);
    return s;
}

// ****************************************************************************
// Function: EndVar
//
// Purpose: 
//   Back out of Silo subdirectories when we're done writing a variable.
//
// Arguments:
//   dbfile : The Silo file.
//   nlevels : The number of levels to back up.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 6 15:34:57 PST 2009
//
// Modifications:
//   
// ****************************************************************************

static void
EndVar(DBfile *dbfile, int nlevels)
{
    for(int i = 0; i < nlevels; ++i)
        DBSetDir(dbfile, "..");
}

// ****************************************************************************
// Function: AbsoluteName 
//
// Purpose: 
//   Prepends a '/' on the name when the number of levels > 0. This is good for
//   determining a mesh name for a variable.
//
// Arguments:
//   name    : The name to decorate.
//   nlevels : The number of levels.
//  
// Returns:    The name to use
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 6 15:40:00 PST 2009
//
// Modifications:
//   
// ****************************************************************************

static string
AbsoluteName(const string &name, int nlevels)
{
    return (nlevels > 0) ? (string("/") + name) : name;
}

// ****************************************************************************
// Function: SaveAndSetSiloLibState / RestoreSiloLibState
//
// Purpose: Manage Silo library global state 
//
// Programmer: Mark C. Miller
// Creation:   May 8, 2009
//
// Modifications:
//    Mark C. Miller, Thu May 21 13:20:42 PDT 2009
//    Worked around bug in Silo-4.7 where once compression is set, it can
//    never be unset.
// ****************************************************************************
static int oldFriendlyHDFNames = 0;
static int oldCheckSums = 0;
static string oldCompressionParams;

//
// This structure is really part of Silo but it is private to Silo. The
// only reason we can actually manipulate it at all is that it is NOT
// defined in Silo as a static structure in a given source file. We got
// a bit lucky in this regard as the SILO_Globals structure is shared
// among Silo's drivers and so can't be static. Nonetheless, its type
// is not known outside of Silo and so we wind up (hackishly) having
// to duplicate the type, here. We need this structure ONLY for Silo
// version 4.7(.0) to work around a bug in that if enableCompression
// is ever set to non-zero, it can never be reset. This then prevents
// DBCreate from working with PDB driver because that driver is
// designed to detect attempts to apply compression and fail if so.
//
#if defined(SILO_VERSION_GE)
#if SILO_VERSION_GE(4,7,0) && !SILO_VERSION_GE(4,7,1)
typedef struct SILO_Globals_copy_t {
    long dataReadMask;
    int allowOverwrites;
    int enableChecksums;
    int enableCompression;
    int enableFriendlyHDF5Names;
    int enableGrabDriver;
    int maxDeprecateWarnings;
} SILO_Globals_copy_t;
extern SILO_Globals_copy_t SILO_Globals;
#endif
#endif

static void
SaveAndSetSiloLibState(int driver, bool checkSums, string compressionParams)
{
    int myRank = PAR_Rank();
#ifdef E_CHECKSUM
    if (driver == DB_HDF5)
        oldCheckSums = DBSetEnableChecksums(checkSums);
    else
    {
        if (checkSums && !myRank)
            cerr << "Checksums supported only on HDF5 driver" << endl;
        oldCheckSums = DBSetEnableChecksums(0);
    }
#endif

#if defined(SILO_VERSION_GE)
#if SILO_VERSION_GE(4,7,0)
    oldCompressionParams = string(DBGetCompression()?DBGetCompression():"");
    if (driver == DB_HDF5)
    {
        oldFriendlyHDFNames = DBSetFriendlyHDF5Names(1);
        DBSetCompression(compressionParams.c_str());
    }
    else
    {

        if (compressionParams != "" && !myRank)
            cerr << "Compression supported only on HDF5 driver" << endl;
        oldFriendlyHDFNames = DBSetFriendlyHDF5Names(0);
#if !SILO_VERSION_GE(4,7,1)
        DBSetCompression("");
        // Hack to work-around bug in Silo lib
        SILO_Globals.enableCompression = 0;
#else
        DBSetCompression(0);
#endif
    }
#endif
#endif
}

static void
RestoreSiloLibState()
{
#ifdef E_CHECKSUM
    DBSetEnableChecksums(oldCheckSums);
#endif

#if defined(SILO_VERSION_GE)
#if SILO_VERSION_GE(4,7,0)
    DBSetFriendlyHDF5Names(oldFriendlyHDFNames);
#if !SILO_VERSION_GE(4,7,1)
    DBSetCompression(oldCompressionParams.c_str());
    // Hack to work-around bug in Silo lib
    SILO_Globals.enableCompression = 0;
#else
    if (oldCompressionParams == "")
        DBSetCompression(0);
    else
        DBSetCompression(oldCompressionParams.c_str());
#endif
#endif
#endif
}

// ****************************************************************************
//  Method: avtSiloWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 09:13:03 PST 2004
//    Added initialization of headerDbMd
//
//    Jeremy Meredith, Tue Mar 27 15:09:30 EDT 2007
//    Initialize nblocks and meshtype -- these were added because we
//    cannot trust what was in the metadata.
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Mark C. Miller, Tue Mar 17 18:09:10 PDT 2009
//    Use const char option names defined in avtSiloOptions.h
//
//    Mark C. Miller, Fri May  8 17:04:15 PDT 2009
//    Added compression and checksum options. Added call to save and set
//    silo lib's global state.
// ****************************************************************************

avtSiloWriter::avtSiloWriter(DBOptionsAttributes *dbopts)
{
    headerDbMd = 0;
    optlist = 0;
    nblocks = 0;
    driver = DB_PDB;
    singleFile = false;
    checkSums = false;

    for (int i = 0; dbopts != 0 && i < dbopts->GetNumberOfOptions(); ++i)
    {
        if (dbopts->GetName(i) == SILO_WROPT_DRIVER)
        {
            switch (dbopts->GetEnum(SILO_WROPT_DRIVER))
            {
                case 0: driver = DB_PDB; break;
                case 1: driver = DB_HDF5; break;
            }
        }
        else if (dbopts->GetName(i) == SILO_WROPT_SINGLE_FILE)
        {
            singleFile = dbopts->GetBool(SILO_WROPT_SINGLE_FILE);
#if defined(PARALLEL)
            if (singleFile)
            {
                debug1 << "Cannot guarentee single file option from parallel engine. Ignoring." << endl;
                singleFile = false;
            }
#endif
        }
        else if (dbopts->GetName(i) == SILO_WROPT_COMPRESSION)
            compressionParams = dbopts->GetString(SILO_WROPT_COMPRESSION);
        else if (dbopts->GetName(i) == SILO_WROPT_CKSUMS)
            checkSums = dbopts->GetBool(SILO_WROPT_CKSUMS);
        else
            debug1 << "Ignoring unknown option \"" << dbopts->GetName(i) << "\"" << endl;
    }

    meshtype = AVT_UNKNOWN_MESH;

    SaveAndSetSiloLibState(driver, checkSums, compressionParams);
}


// ****************************************************************************
//  Method: avtSiloWriter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Fri May  8 17:05:03 PDT 2009
//    Added call to restore silo lib's global state.
// ****************************************************************************

avtSiloWriter::~avtSiloWriter()
{
    RestoreSiloLibState();

    if (optlist != NULL)
    {
        DBFreeOptlist(optlist);
        optlist = NULL;
    }
}


// ****************************************************************************
//  Method: avtSiloWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the Op name for the files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 27 15:10:21 EDT 2007
//    Added nblocks to this function and save it so we don't have to 
//    trust the meta data.
//
//    Cyrus Harrison, Thu Aug 16 20:26:28 PDT 2007
//    Separate dir and file name, so only file name can be used as stem 
//    for mesh and var names.
//
// ****************************************************************************

void
avtSiloWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
    dir ="";
    // find dir if provided
    int idx = stem.rfind("/");
    if ( idx != string::npos )
    {
        int stem_len = stem.size() - (idx+1) ;
        dir  = stem.substr(0,idx+1);
        stem = stem.substr(idx+1,stem_len);
    }
}


// ****************************************************************************
//  Method: avtSiloWriter::WriteHeaders
//
//  Purpose:
//      This will write out the multi-vars for the Silo constructs.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 09:13:03 PST 2004
//    Moved bulk of code to CloseFile. Stored away args in data members for
//    eventual use in CloseFile
//
//    Jeremy Meredith, Tue Mar 27 15:10:43 EDT 2007
//    Added meshtype as a member variable, and only start with the initial
//    guess from the mesh meta-data.
//
// ****************************************************************************

void
avtSiloWriter::WriteHeaders(const avtDatabaseMetaData *md,
                            vector<string> &scalars, vector<string> &vectors,
                            vector<string> &materials)
{
    const avtMeshMetaData *mmd = md->GetMesh(0);
    meshname = mmd->name;
    matname = (materials.size() > 0 ? materials[0] : "");

    // store args away for eventual use in CloseFile
    headerDbMd      = md;
    headerScalars   = scalars;
    headerVectors   = vectors;
    headerMaterials = materials;

    // Initial guess for mesh type.  Note that we're assuming there is
    // one mesh type for all domains, but it's possible this is not
    // always true.  Parallel unification of the actual chunks written
    // out is needed to solve this correctly.
    meshtype = mmd->meshType;

    ConstructChunkOptlist(md);
}


// ****************************************************************************
//  Method: avtSiloWriter::ConstructMultimesh
//
//  Purpose:
//      Constructs a multi-mesh based on meta-data.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 12:17:28 PST 2004
//    Added code to output spatial extents/zone counts
//
//    Mark C. Miller, Tue Oct  5 12:41:41 PDT 2004
//    Changed to use test for DBOPT_EXTENTS_SIZE for code that adds extents
//    options
//
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Use the saved mesh type and number of blocks instead of assuming
//    the metadata was correct.
//
//    Mark C. Miller, Mon Jun  4 17:29:11 PDT 2007
//    Don't write extents if running in parallel. 
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 15:00:31 PST 2009
//    Allow subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::ConstructMultimesh(DBfile *dbfile, const avtMeshMetaData *mmd)
{
    int   i,j,k;

    int nlevels = 0;
    string meshName = BeginVar(dbfile, mmd->name, nlevels);

    //
    // Determine what the Silo type is.
    //
    int *meshtypes = new int[nblocks];
    int silo_mesh_type = DB_INVALID_OBJECT;
    switch (meshtype)
    {
      case AVT_RECTILINEAR_MESH:
      case AVT_CURVILINEAR_MESH:
        silo_mesh_type = DB_QUADMESH;
        break;

      case AVT_UNSTRUCTURED_MESH:
      case AVT_SURFACE_MESH:
      case AVT_POINT_MESH:
        silo_mesh_type = DB_UCDMESH;
        break;

      default:
        silo_mesh_type = DB_INVALID_OBJECT;
        break;
    }

    if (hasMaterialsInProblem && !mustGetMaterialsAdditionally)
       silo_mesh_type = DB_UCDMESH;  // After we do MIR.

    avtDataAttributes& atts = GetInput()->GetInfo().GetAttributes();
    int ndims = atts.GetSpatialDimension();

    //
    // Construct the names for each mesh.
    //
    char **mesh_names = new char*[nblocks];
    double *extents = new double[nblocks * ndims * 2];
    k = 0;
    for (i = 0 ; i < nblocks ; i++)
    {
        meshtypes[i] = silo_mesh_type;
        char tmp[1024];
        if (singleFile)
        {
            if(nlevels > 0)
                sprintf(tmp, "/domain_%d/%s", i, VN(mmd->name));
            else
                sprintf(tmp, "domain_%d/%s", i, VN(mmd->name));
        }
        else
            sprintf(tmp, "%s.%d.silo:/%s", stem.c_str(), i,VN(mmd->name));
        mesh_names[i] = new char[strlen(tmp)+1];
        strcpy(mesh_names[i], tmp);

        for (j = 0; j < ndims; j++)
           extents[k++] = spatialMins[i*ndims+j];
        for (j = 0; j < ndims; j++)
           extents[k++] = spatialMaxs[i*ndims+j];
    }

    //
    // Build zone-counts array
    //
    int *zone_counts = new int[nblocks];
    for (i = 0; i < nblocks; i++)
        zone_counts[i] = zoneCounts[i];

    DBoptlist *tmpOptlist = DBMakeOptlist(20);

    int cycle = atts.GetCycle();
    DBAddOption(tmpOptlist, DBOPT_CYCLE, &cycle);
    float ftime = atts.GetTime();
    DBAddOption(tmpOptlist, DBOPT_TIME, &ftime);
    double dtime = atts.GetTime();
    DBAddOption(tmpOptlist, DBOPT_DTIME, &dtime);
    if (atts.GetXUnits() != "")
        DBAddOption(tmpOptlist, DBOPT_XUNITS, (char *) atts.GetXUnits().c_str());
    if (atts.GetYUnits() != "")
        DBAddOption(tmpOptlist, DBOPT_YUNITS, (char *) atts.GetYUnits().c_str());
    if (atts.GetZUnits() != "")
        DBAddOption(tmpOptlist, DBOPT_ZUNITS, (char *) atts.GetZUnits().c_str());

    // the following silo options exist only for silo 4.4 and later
#if defined(DBOPT_EXTENTS_SIZE) && !defined(PARALLEL)
    int extsize = ndims * 2;
    DBAddOption(tmpOptlist, DBOPT_EXTENTS_SIZE, &extsize);
    DBAddOption(tmpOptlist, DBOPT_EXTENTS, extents);
    DBAddOption(tmpOptlist, DBOPT_ZONECOUNTS, zone_counts);
#endif

    //
    // Write the actual header information.
    //
    DBPutMultimesh(dbfile, (char *) meshName.c_str(),
        nblocks, mesh_names, meshtypes, tmpOptlist);

    //
    // Clean up memory.
    //
    DBFreeOptlist(tmpOptlist);
    for (i = 0 ; i < nblocks ; i++)
    {
        delete [] mesh_names[i];
    }
    delete [] mesh_names;
    delete [] meshtypes;
    delete [] extents;
    delete [] zone_counts;

    EndVar(dbfile, nlevels);
}


// ****************************************************************************
//  Method: avtSiloWriter::ConstructMultivar
//
//  Purpose:
//      Constructs a multi-var based on meta-data.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Oct  5 12:41:41 PDT 2004
//    Changed code that outputs extents options to use DBOPT_EXTENTS_SIZE for
//    conditional compilation
//
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Use the saved mesh type and number of blocks instead of assuming
//    the metadata was correct.
//
//    Mark C. Miller, Mon Jun  4 17:29:11 PDT 2007
//    Don't write extents if running in parallel. 
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 15:00:31 PST 2009
//    Allow subdirectories. I also fixed a bug with adding extentsize to the
//    option list. The address of a temporary variable was taken and then
//    it went out of scope.
//
// ****************************************************************************

void
avtSiloWriter::ConstructMultivar(DBfile *dbfile, const string &sname,
                                 const avtMeshMetaData *mmd)
{
    int   i,j,k;

    //
    // Begin the variable
    //
    int nlevels = 0;
    string varName = BeginVar(dbfile, sname, nlevels);

    //
    // Determine what the Silo type is.
    //
    int *vartypes = new int[nblocks];
    int silo_var_type = DB_INVALID_OBJECT;
    switch (meshtype)
    {
      case AVT_RECTILINEAR_MESH:
      case AVT_CURVILINEAR_MESH:
        silo_var_type = DB_QUADVAR;
        break;

      case AVT_UNSTRUCTURED_MESH:
      case AVT_SURFACE_MESH:
      case AVT_POINT_MESH:
        silo_var_type = DB_UCDVAR;
        break;

      default:
        silo_var_type = DB_INVALID_OBJECT;
        break;
    }

    if (hasMaterialsInProblem && !mustGetMaterialsAdditionally)
       silo_var_type = DB_UCDVAR;  // After we do MIR.

    //
    // lookup extents for this variable and setup the info we'll need
    //
    double *extents = 0;
    int ncomps = 0;
    vector<double> minVals;
    vector<double> maxVals;
    std::map<string, vector<double> >::iterator minsMap;
    minsMap = dataMins.find(sname);
    if (minsMap != dataMins.end())
    {
        minVals = minsMap->second;
        maxVals = dataMaxs[sname];
        ncomps = minVals.size() / nblocks; 
        extents = new double[nblocks * ncomps * 2];
    }

    //
    // Construct the names for each var.
    //
    k = 0;
    char **var_names = new char*[nblocks];
    for (i = 0 ; i < nblocks ; i++)
    {
        vartypes[i] = silo_var_type;
        char tmp[1024];
        if (singleFile)
        {
            if(nlevels > 0)
                sprintf(tmp, "/domain_%d/%s", i, VN(sname));
            else
                sprintf(tmp, "domain_%d/%s", i, VN(sname));
        }
        else
            sprintf(tmp, "%s.%d.silo:/%s", stem.c_str(), i, VN(sname));
        var_names[i] = new char[strlen(tmp)+1];
        strcpy(var_names[i], tmp);

        if (extents != 0)
        {
            for (j = 0; j < ncomps; j++)
                extents[k++] = minVals[i*ncomps+j];
            for (j = 0; j < ncomps; j++)
                extents[k++] = maxVals[i*ncomps+j];
        }
    }

    DBoptlist *tmpOptlist = DBMakeOptlist(20);

    avtDataAttributes& atts = GetInput()->GetInfo().GetAttributes();
    int cycle = atts.GetCycle();
    DBAddOption(tmpOptlist, DBOPT_CYCLE, &cycle);
    float ftime = atts.GetTime();
    DBAddOption(tmpOptlist, DBOPT_TIME, &ftime);
    double dtime = atts.GetTime();
    DBAddOption(tmpOptlist, DBOPT_DTIME, &dtime);
    if (atts.GetXUnits() != "")
        DBAddOption(tmpOptlist, DBOPT_XUNITS, (char *) atts.GetXUnits().c_str());
    if (atts.GetYUnits() != "")
        DBAddOption(tmpOptlist, DBOPT_YUNITS, (char *) atts.GetYUnits().c_str());
    if (atts.GetZUnits() != "")
        DBAddOption(tmpOptlist, DBOPT_ZUNITS, (char *) atts.GetZUnits().c_str());

    // the following silo options exist only for silo 4.4 and later
    int extsize = ncomps * 2;
    if (extents != 0)
    {
#if defined(DBOPT_EXTENTS_SIZE) && !defined(PARALLEL)
        DBAddOption(tmpOptlist, DBOPT_EXTENTS_SIZE, &extsize);
        DBAddOption(tmpOptlist, DBOPT_EXTENTS, extents);
#endif
    }

    //
    // Write the actual header information.
    //
    DBPutMultivar(dbfile, (char *) varName.c_str(),
        nblocks, var_names, vartypes, tmpOptlist);

    //
    // Clean up memory.
    //
    DBFreeOptlist(tmpOptlist);
    for (i = 0 ; i < nblocks ; i++)
    {
        delete [] var_names[i];
    }
    delete [] var_names;
    delete [] vartypes;
    if (extents != 0)
        delete [] extents;

    //
    // End the variable
    //
    EndVar(dbfile, nlevels);
}


// ****************************************************************************
//  Method: avtSiloWriter::ConstructMultimat
//
//  Purpose:
//      Constructs a multimat based on meta-data.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Use the saved mesh type and number of blocks instead of assuming
//    the metadata was correct.
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 15:00:31 PST 2009
//    Allow subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::ConstructMultimat(DBfile *dbfile, const string &mname,
                                 const avtMeshMetaData *mmd)
{
    int   i;

    int nlevels = 0;
    string matName = BeginVar(dbfile, mname, nlevels);

    //
    // Construct the names for each mat
    //
    char **mat_names = new char*[nblocks];
    for (i = 0 ; i < nblocks ; i++)
    {
        char tmp[1024];
        if (singleFile)
        {
            if(nlevels > 0)
                sprintf(tmp, "/domain_%d/%s", i, VN(mname));
            else
                sprintf(tmp, "domain_%d/%s", i, VN(mname));
        }
        else
            sprintf(tmp, "%s.%d.silo:/%s", stem.c_str(), i, VN(mname));
        mat_names[i] = new char[strlen(tmp)+1];
        strcpy(mat_names[i], tmp);
    }

    //
    // Write the actual header information.
    //
    DBPutMultimat(dbfile, (char *) matName.c_str(), nblocks, mat_names, optlist);

    //
    // Clean up memory.
    //
    for (i = 0 ; i < nblocks ; i++)
    {
        delete [] mat_names[i];
    }
    delete [] mat_names;

    EndVar(dbfile, nlevels);
}


// ****************************************************************************
//  Method: avtSiloWriter::ConstructChunkOptlist
//
//  Purpose:
//      Constructs an optlist from the input dataset and the database 
//      meta-data.
//
//  Programmer: Hank CHilds
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 18:09:45 PST 2004
//    Made the local vars referenced by optlist static so they live outside
//    of this call. DBAddOption doesn't copy them. It just records the pointer
//
//    Brad Whitlock, Tue May 19 12:17:48 PDT 2009
//    Make sure that we save out the cellOrigin.
//
// ****************************************************************************

void
avtSiloWriter::ConstructChunkOptlist(const avtDatabaseMetaData *md)
{
    optlist = DBMakeOptlist(10);
    
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    static int cycle = atts.GetCycle();
    DBAddOption(optlist, DBOPT_CYCLE, &cycle);
    static float ftime = atts.GetTime();
    DBAddOption(optlist, DBOPT_TIME, &ftime);
    static double dtime = atts.GetTime();
    DBAddOption(optlist, DBOPT_DTIME, &dtime);
    static int cellOrigin = atts.GetCellOrigin();
    DBAddOption(optlist, DBOPT_ORIGIN, &cellOrigin);
    if (atts.GetXUnits() != "")
        DBAddOption(optlist, DBOPT_XUNITS, (char *) atts.GetXUnits().c_str());
    if (atts.GetYUnits() != "")
        DBAddOption(optlist, DBOPT_YUNITS, (char *) atts.GetYUnits().c_str());
    if (atts.GetZUnits() != "")
        DBAddOption(optlist, DBOPT_ZUNITS, (char *) atts.GetZUnits().c_str());
}


// ****************************************************************************
//  Method: avtSiloWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Jun 13 10:22:35 PDT 2006
//    Added call to temporarily disable checksums in Silo just in case they
//    might have been enabled, since PDB driver can't do checksumming.
//    Nonetheless, PDB only checks during file creation and otherwise silently
//    ignores the setting.
//
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Save off the actual mesh type we encountered, as this may differ
//    from what was in the metadata.
//
//    Hank Childs, Wed Mar 28 10:12:01 PDT 2007
//    Name the file differently for single block.
//
//    Cyrus Harrison, Thu Aug 16 20:42:30 PDT 2007
//    Use dir+stem to create filename.
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Mark C. Miller, Thu Aug  7 23:38:24 PDT 2008
//    Fixed filename generation for single block case
//
//    Mark C. Miller, Mon Sep 22 22:03:55 PDT 2008
//    Fixed single file, single block case to prevent it from making
//    "domain" subdirs.
//
//    Mark C. Miller, Fri May  8 17:05:47 PDT 2009
//    Moved work dealing with Silo lib's global behavior to
//    SaveAndSetSiloLibState.
// ****************************************************************************

void
avtSiloWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    //
    // Now matter what mesh type we have, the file they should go into should
    // have the same name.  Set up that file now.
    //
    string fname = dir + stem;
    char filename[1024];
    if (singleFile || nblocks == 1)
        sprintf(filename, "%s.silo", fname.c_str());
    else
        sprintf(filename, "%s.%d.silo", fname.c_str(), chunk);


    DBfile *dbfile;
    if (chunk > 0 && singleFile)
    {
        dbfile = DBOpen(filename, DB_UNKNOWN, DB_APPEND);

        char dirname[32];
        sprintf(dirname, "domain_%d", chunk);
        DBMkDir(dbfile, dirname);
        DBSetDir(dbfile, dirname);
    }
    else
    {
        dbfile = DBCreate(filename, 0, DB_LOCAL, 
                     "Silo file written by VisIt", driver);

        if (singleFile && nblocks > 1)
        {
            char dirname[32];
            sprintf(dirname, "domain_%d", chunk);
            DBMkDir(dbfile, dirname);
            DBSetDir(dbfile, dirname);
        }
    }

    //
    // Use sub-routines to do the mesh-type specific writes.
    //
    switch (ds->GetDataObjectType())
    {
       case VTK_UNSTRUCTURED_GRID:
         meshtype = AVT_UNSTRUCTURED_MESH;
         WriteUnstructuredMesh(dbfile, (vtkUnstructuredGrid *) ds, chunk);
         break;

       case VTK_STRUCTURED_GRID:
         meshtype = AVT_CURVILINEAR_MESH;
         WriteCurvilinearMesh(dbfile, (vtkStructuredGrid *) ds, chunk);
         break;

       case VTK_RECTILINEAR_GRID:
         meshtype = AVT_RECTILINEAR_MESH;
         WriteRectilinearMesh(dbfile, (vtkRectilinearGrid *) ds, chunk);
         break;

       case VTK_POLY_DATA:
         meshtype = AVT_UNSTRUCTURED_MESH;
         WritePolygonalMesh(dbfile, (vtkPolyData *) ds, chunk);
         break;

       default:
         EXCEPTION0(ImproperUseException);
    }

    DBClose(dbfile);
}


// ****************************************************************************
//  Method: avtSiloWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifcations:
//    Mark C. Miller, Tue Mar  9 09:13:03 PST 2004
//    Relocated bulk of WriteHeader code to here
//
//    Hank Childs, Wed Jun 14 14:05:19 PDT 2006
//    Added the changes done on June 13th by Mark Miller for "WriteChunk" to
//    this method as well.  The following are Mark's comments:
//    Added call to temporarily disable checksums in Silo just in case they
//    might have been enabled, since PDB driver can't do checksumming.
//    Nonetheless, PDB only checks during file creation and otherwise silently
//    ignores the setting.
//
//    Hank Childs, Wed Mar 28 10:19:18 PDT 2007
//    Don't write a root file if there is only one block.
//
//    Cyrus Harrison, Fri Aug  3 20:53:34 PDT 2007
//    Use first processor with data to write the root file.
//    (processor 0 may not contain data and b/c of this have invalid metadata)
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Brad Whitlock, Wed Mar 11 11:10:14 PDT 2009
//    Added code to write expressions that came from the database back out
//    to the master file.
//
//    Mark C. Miller, Fri May  8 17:05:47 PDT 2009
//    Moved work dealing with Silo lib's global behavior to
//    SaveAndSetSiloLibState.
// ****************************************************************************

void
avtSiloWriter::CloseFile(void)
{
    int i;
 
    const avtMeshMetaData *mmd = headerDbMd->GetMesh(0);

    // free the optlist
    if (optlist != 0)
    {
        DBFreeOptlist(optlist);
        optlist = 0;
    }

    if (nblocks == 1)
    {
        return;
    }

    int nprocs = PAR_Size();
    int rootid = nprocs + 1;
    int procid = PAR_Rank();

    // get number of of datasets local to this processor
    avtDataTree_p rootnode = GetInputDataTree();
    int nchunks = rootnode->GetNumberOfLeaves();

    // find the lowest ranked processor with data
    if (nchunks > 0)
        rootid = procid;

    // find min rootid
    bool root = ThisProcessorHasMinimumValue((double)rootid);

    if (root)
    {
        debug5 << "avtSiloWriter: proc " << procid << " writing silo root"
               << "file" << endl;
        char filename[1024];
        string fname = dir + stem;
        sprintf(filename, "%s.silo", fname.c_str());


        DBfile *dbfile;
        if (singleFile)
            dbfile = DBOpen(filename, DB_UNKNOWN, DB_APPEND);
        else
            dbfile = DBCreate(filename, 0, DB_LOCAL, 
                         "Silo file written by VisIt", driver);

        ConstructMultimesh(dbfile, mmd);
        for (i = 0 ; i < headerScalars.size() ; i++)
            ConstructMultivar(dbfile, headerScalars[i], mmd);
        for (i = 0 ; i < headerVectors.size() ; i++)
            ConstructMultivar(dbfile, headerVectors[i], mmd);
        for (i = 0 ; i < headerMaterials.size() ; i++)
            ConstructMultimat(dbfile, headerMaterials[i], mmd);
        WriteExpressions(dbfile);
        DBClose(dbfile);
    }
}


// ****************************************************************************
//  Method: avtSiloWriter::WriteUnstructuredMesh
//
//  Purpose:
//      Writes out an unstructured mesh.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 09:31:21 PST 2004
//    Added code to compute and store spatial extents and zone counts
//    for this chunk
//
//    Hank Childs, Sun Feb 13 13:19:07 PST 2005
//    Re-order nodes for pixels.
//
//    Cyrus Harrison, Fri Aug  3 20:46:47 PDT 2007
//    Re-order nodes for tets.
//
//    Cyrus Harrison, Tue Sep 11 10:16:11 PDT 2007
//    Fixed node order for wedges.
//
//    Hank Childs, Fri Feb  1 09:15:25 PST 2008
//    Re-order nodes for voxels.
//
//    Cyrus Harrison, Tue Feb 26 17:42:45 PST 2008
//    Replaced deprecated DBPutZoneList call with  DBPutZoneList2 call
//
//    Mark C. Miller, Wed Jul 23 17:48:21 PDT 2008
//    Added code to detect and output silo point meshes
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 10:06:49 PDT 2009
//    Allow for subdirectories.
//
//    Brad Whitlock, Tue May 19 11:23:06 PDT 2009
//    Account for ghost zones so they don't get saved out as real zones.
//
// ****************************************************************************

void
avtSiloWriter::WriteUnstructuredMesh(DBfile *dbfile, vtkUnstructuredGrid *ug,
                                    int chunk)
{
    int  i, j;

    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int npts = ug->GetNumberOfPoints();
    int nzones = ug->GetNumberOfCells();

    //
    // Construct the coordinates.
    //
    float *coords[3] = { NULL, NULL, NULL };
    vtkPoints *vtk_pts = ug->GetPoints();
    float *vtk_ptr = (float *) vtk_pts->GetVoidPointer(0);
    for (i = 0 ; i < dim ; i++)
    {
        double dimMin = +DBL_MAX;
        double dimMax = -DBL_MAX;
        coords[i] = new float[npts];
        for (j = 0 ; j < npts ; j++)
        {
            coords[i][j] = vtk_ptr[3*j+i];
            if (coords[i][j] < dimMin)
                dimMin = coords[i][j];
            if (coords[i][j] > dimMax)
                dimMax = coords[i][j];
        }
        spatialMins.push_back(dimMin);
        spatialMaxs.push_back(dimMax);
    }
    zoneCounts.push_back(nzones);

    // Get the ghost zone array, if there is one.
    bool hasGhostZones = false;
    vtkDataArray *ghostZones = ug->GetCellData()->GetArray("avtGhostZones");
    unsigned char *gz = 0;
    if(ghostZones != 0)
        gz = (unsigned char *)ghostZones->GetVoidPointer(0);

    //
    // Put the zone list into a digestable form for Silo.
    //
    vector<int> shapetype;
    vector<int> shapecnt;
    vector<int> shapesize;
    vector<int> zonelist;
    int         npointcells = 0;
    int         nPasses = (gz != 0) ? 2 : 1;
    int         realZones = 0;
    for(int pass = 0; pass < nPasses; ++pass)
    {
        for (i = 0 ; i < nzones ; i++)
        {
            if(pass == 0)
            {
                if(gz != 0 && gz[i] > 0)
                {
                    hasGhostZones = true;
                    // Skip this ghost zone.
                    continue;
                }

                ++realZones;
            }
            else
            {
                if(gz[i] == 0)
                {
                    // Skip this real zone.
                    continue;
                }
            }

            //
            // Get the i'th cell and put its connectivity info into the 'zonelist'
            // array.
            //
            vtkCell *cell = ug->GetCell(i);
            for (j = 0 ; j < cell->GetNumberOfPoints() ; j++)
            {
                zonelist.push_back(cell->GetPointId(j));
            }

            //
            // Silo keeps track of how many of each shape it has in a row.
            // (This means that it doesn't have to explicitly store the size
            // of each individual cell).  Maintain that information.
            //
            int lastshape = shapesize.size()-1;
            int thisshapesize = cell->GetNumberOfPoints();
            if (lastshape >= 0 && (thisshapesize == shapesize[lastshape]))
                shapecnt[lastshape]++;
            else
            {
                int silo_type = VTKZoneTypeToSiloZoneType(cell->GetCellType());
                shapetype.push_back(silo_type);
                shapesize.push_back(thisshapesize);
                shapecnt.push_back(1);  // 1 is the # of shapes we have seen with
                                        // this size ie the one we are processing.
            }

            // keep count to check if all cells are in fact point cells
            if (cell->GetCellType() == VTK_VERTEX)
            {
                npointcells++;
            }

            //
            // Wedges, pyramids, and tets have a different ordering in Silo and VTK.
            // Make the corrections for these cases.
            //
            if (dim == 2 && (cell->GetCellType() == VTK_PIXEL))
            {
                int startOfZone = zonelist.size() - 4;
                int tmp = zonelist[startOfZone+2];
                zonelist[startOfZone+2] = zonelist[startOfZone+3];
                zonelist[startOfZone+3] = tmp;
            }
            if (dim == 3 && (cell->GetCellType() == VTK_TETRA))
            {
                int startOfZone = zonelist.size() - 4;
                int tmp = zonelist[startOfZone];
                zonelist[startOfZone]   = zonelist[startOfZone+1];
                zonelist[startOfZone+1] = tmp;
            }
            if (dim == 3 && (cell->GetNumberOfPoints() == 6))
            {
                int startOfZone = zonelist.size() - 6;
                int tmp = zonelist[startOfZone+5];
                zonelist[startOfZone+5] = zonelist[startOfZone+2];
                zonelist[startOfZone+2] = zonelist[startOfZone];
                zonelist[startOfZone]   = zonelist[startOfZone+4];
                zonelist[startOfZone+4] = tmp;
            }
            if (dim == 3 && (cell->GetNumberOfPoints() == 5))
            {
                int startOfZone = zonelist.size() - 5;
                int tmp = zonelist[startOfZone+1];
                zonelist[startOfZone+1] = zonelist[startOfZone+3];
                zonelist[startOfZone+3] = tmp;
            }
            if (dim == 3 && (cell->GetCellType() == VTK_VOXEL))
            {
                int startOfZone = zonelist.size() - 8;
                int tmp = zonelist[startOfZone+2];
                zonelist[startOfZone+2] = zonelist[startOfZone+3];
                zonelist[startOfZone+3] = tmp;
                tmp = zonelist[startOfZone+6];
                zonelist[startOfZone+6] = zonelist[startOfZone+7];
                zonelist[startOfZone+7] = tmp;
            }
        }
    }
    
    int nlevels = 0;
    string meshName = BeginVar(dbfile, meshname, nlevels);

    if (npointcells == nzones && npointcells == npts)
    {
        DBPutPointmesh(dbfile, meshName.c_str(), dim, coords,
            npts, DB_FLOAT, optlist);
    }
    else
    {
        //
        // Now write out the zonelist to the file.
        //
        int *zl = &(zonelist[0]);
        int lzl = zonelist.size();
        int *st = &(shapetype[0]);
        int *ss = &(shapesize[0]);
        int *sc = &(shapecnt[0]);
        int lo_offset = 0;
        int hi_offset = hasGhostZones ? (nzones - realZones) : 0;
        int nshapes = shapesize.size();
        DBPutZonelist2(dbfile, "zonelist", nzones, dim, zl, lzl, 
                       0, lo_offset, hi_offset, st, ss, sc,nshapes, NULL);
    
        //
        // Now write the actual mesh.
        //
        DBPutUcdmesh(dbfile, (char *) meshName.c_str(), dim, NULL, coords, npts,
                     nzones, "zonelist", NULL, DB_FLOAT, optlist);
    }
    EndVar(dbfile, nlevels);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];

    WriteUcdvars(dbfile, ug->GetPointData(), ug->GetCellData(),
                 npointcells == nzones, hasGhostZones ? gz : 0);
    if(hasGhostZones)
    {
        debug1 << "TODO: The materials need to be reordered for this ucdmesh because "
                  "there are ghost zones." << endl;
    }
    WriteMaterials(dbfile, ug->GetCellData(), chunk);
}


// ****************************************************************************
//  Method: avtSiloWriter::WriteCurvilinearMesh
//
//  Purpose:
//      Writes out an curvilinear mesh.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue Mar  9 09:31:21 PST 2004
//    Added code to compute and store spatial extents and zone counts
//    for this chunk
//
//    Hank Childs, Fri Oct  5 09:13:56 PDT 2007
//    Fix some logic in setting up zone count.
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 10:06:49 PDT 2009
//    Allow for subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::WriteCurvilinearMesh(DBfile *dbfile, vtkStructuredGrid *sg,
                                    int chunk)
{
    int  i, j;

    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    //
    // Construct the coordinates.
    //
    float *coords[3] = { NULL, NULL, NULL };
    vtkPoints *vtk_pts = sg->GetPoints();
    float *vtk_ptr = (float *) vtk_pts->GetVoidPointer(0);
    int npts  = sg->GetNumberOfPoints();
    for (i = 0 ; i < ndims ; i++)
    {
        double dimMin = +DBL_MAX;
        double dimMax = -DBL_MAX;
        coords[i] = new float[npts];
        for (j = 0 ; j < npts ; j++)
        {
            coords[i][j] = vtk_ptr[3*j+i];
            if (coords[i][j] < dimMin)
                dimMin = coords[i][j];
            if (coords[i][j] > dimMax)
                dimMax = coords[i][j];
        }
        spatialMins.push_back(dimMin);
        spatialMaxs.push_back(dimMax);
    }

    int dims[3];
    sg->GetDimensions(dims);
    int nzones = 1;
    for (i = 0 ; i < ndims ; i++)
        if (dims[i] > 1)
            nzones *= (dims[i]-1);
    zoneCounts.push_back(nzones);

    //
    // Write the curvilinear mesh to the Silo file.
    //
    int nlevels = 0;
    string meshName = BeginVar(dbfile, meshname, nlevels);
    DBPutQuadmesh(dbfile, (char *) meshName.c_str(), NULL, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, optlist);
    EndVar(dbfile, nlevels);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];

    WriteQuadvars(dbfile, sg->GetPointData(), sg->GetCellData(),ndims,dims);
    WriteMaterials(dbfile, sg->GetCellData(), chunk);
}


// ****************************************************************************
//  Method: avtSiloWriter::WriteRectilinearMesh
//
//  Purpose:
//      Writes out an rectilinear mesh.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue Mar  9 09:31:21 PST 2004
//    Added code to compute and store spatial extents and zone counts
//    for this chunk
//
//    Hank Childs, Fri Oct  5 09:13:56 PDT 2007
//    Fix some logic in setting up zone count.
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 10:06:49 PDT 2009
//    Allow for subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::WriteRectilinearMesh(DBfile *dbfile, vtkRectilinearGrid *rg,
                                    int chunk)
{
    int  i, j;

    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    //
    // Construct the coordinates.
    //
    float *coords[3] = { NULL, NULL, NULL };
    int dims[3];
    rg->GetDimensions(dims);
    vtkDataArray *vtk_coords[3] = { NULL, NULL, NULL };
    if (ndims > 0)
        vtk_coords[0] = rg->GetXCoordinates();
    if (ndims > 1)
        vtk_coords[1] = rg->GetYCoordinates();
    if (ndims > 2)
        vtk_coords[2] = rg->GetZCoordinates();

    for (i = 0 ; i < ndims ; i++)
    {
        if (vtk_coords[i] == NULL)
            continue;

        double dimMin = +DBL_MAX;
        double dimMax = -DBL_MAX;
        int npts = vtk_coords[i]->GetNumberOfTuples();
        coords[i] = new float[npts];
        for (j = 0 ; j < npts ; j++)
        {
            coords[i][j] = vtk_coords[i]->GetTuple1(j);
            if (coords[i][j] < dimMin)
                dimMin = coords[i][j];
            if (coords[i][j] > dimMax)
                dimMax = coords[i][j];
        }
        spatialMins.push_back(dimMin);
        spatialMaxs.push_back(dimMax);
    }

    int nzones = 1;
    for (i = 0 ; i < ndims ; i++)
        if (dims[i] > 1)
            nzones *= (dims[i]-1);
    zoneCounts.push_back(nzones);

    //
    // Write the rectilinear mesh to the Silo file.
    //
    int nlevels = 0;
    string meshName = BeginVar(dbfile, meshname, nlevels);
    DBPutQuadmesh(dbfile, (char *) meshName.c_str(), NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, optlist);
    EndVar(dbfile, nlevels);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];

    WriteQuadvars(dbfile, rg->GetPointData(), rg->GetCellData(),ndims,dims);
    WriteMaterials(dbfile, rg->GetCellData(), chunk);
}


// ****************************************************************************
//  Method: avtSiloWriter::WritePolygonalMesh
//
//  Purpose:
//      Writes out a polygonal mesh.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 09:31:21 PST 2004
//    Added code to compute and store spatial extents and zone counts
//    for this chunk
//
//    Cyrus Harrison, Tue Feb 26 17:42:45 PST 2008
//    Replaced deprecated DBPutZoneList call with  DBPutZoneList2 call
//
//    Mark C. Miller, Wed Jul 23 17:48:21 PDT 2008
//    Added code to detect and output silo point meshes
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 10:06:49 PDT 2009
//    Allow for subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::WritePolygonalMesh(DBfile *dbfile, vtkPolyData *pd,
                                  int chunk)
{
    int  i, j;

    int ndims  = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int npts   = pd->GetNumberOfPoints();
    int nzones = pd->GetNumberOfCells();

    //
    // Construct the coordinates.
    //
    float *coords[3] = { NULL, NULL, NULL };
    vtkPoints *vtk_pts = pd->GetPoints();
    float *vtk_ptr = (float *) vtk_pts->GetVoidPointer(0);
    for (i = 0 ; i < ndims ; i++)
    {
        double dimMin = +DBL_MAX;
        double dimMax = -DBL_MAX;
        coords[i] = new float[npts];
        for (j = 0 ; j < npts ; j++)
        {
            coords[i][j] = vtk_ptr[3*j+i];
            if (coords[i][j] < dimMin)
                dimMin = coords[i][j];
            if (coords[i][j] > dimMax)
                dimMax = coords[i][j];
        }
        spatialMins.push_back(dimMin);
        spatialMaxs.push_back(dimMax);
    }
    zoneCounts.push_back(nzones);

    //
    // We will be writing this dataset out as an unstructured mesh in Silo.
    // So we will need a zonelist.  Construct that now.
    //
    vector<int> shapetype;
    vector<int> shapecnt;
    vector<int> shapesize;
    vector<int> zonelist;
    int         npointcells = 0;
    for (i = 0 ; i < nzones ; i++)
    {
        //
        // Get the i'th cell and put its connectivity info into the 'zonelist'
        // array.
        //
        vtkCell *cell = pd->GetCell(i);
        for (j = 0 ; j < cell->GetNumberOfPoints() ; j++)
        {
            zonelist.push_back(cell->GetPointId(j));
        }

        //
        // Silo keeps track of how many of each shape it has in a row.
        // (This means that it doesn't have to explicitly store the size
        // of each individual cell).  Maintain that information.
        //
        int lastshape = shapesize.size()-1;
        int thisshapesize = cell->GetNumberOfPoints();
        if (lastshape >= 0 && (thisshapesize == shapesize[lastshape]))
            shapecnt[lastshape]++;
        else
        {
            int silo_type = VTKZoneTypeToSiloZoneType(cell->GetCellType());
            shapetype.push_back(silo_type);
            shapesize.push_back(thisshapesize);
            shapecnt.push_back(1);  // 1 is the # of shapes we have seen with
                                    // this size ie the one we are processing.
        }

        // keep track to see if all cells are points
        if (cell->GetCellType() == VTK_VERTEX)
        {
            npointcells++;
        }
    }

    int nlevels = 0;
    string meshName = BeginVar(dbfile, meshname, nlevels);
    if (npointcells == nzones && npointcells == npts)
    {
        DBPutPointmesh(dbfile, (char *) meshName.c_str(), ndims, coords,
            npts, DB_FLOAT, optlist);
    }
    else
    {
        //
        // Now write out the zonelist to the file.
        //
        int *zl = &(zonelist[0]);
        int lzl = zonelist.size();
        int *st = &(shapetype[0]);
        int *ss = &(shapesize[0]);
        int *sc = &(shapecnt[0]);
        int nshapes = shapesize.size();

        DBPutZonelist2(dbfile, "zonelist", nzones, ndims, zl, lzl, 
                       0, 0, 0, st, ss, sc,nshapes, NULL);
        //
        // Now write the actual mesh.
        //
        DBPutUcdmesh(dbfile, (char *) meshName.c_str(), ndims, NULL, coords, npts,
                     nzones, "zonelist", NULL, DB_FLOAT, optlist);
    }
    EndVar(dbfile, nlevels);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];

    WriteUcdvars(dbfile, pd->GetPointData(), pd->GetCellData(),
        npointcells == nzones, 0);
    WriteMaterials(dbfile, pd->GetCellData(), chunk);
}

// ****************************************************************************
// Method: ReorderUcdvarUsingGhostZones
//
// Purpose: 
//   If there are ghost zones then this function creates a copy of the input
//   data array where the data is reordered such that the real zones come
//   first and then the ghost zone data comes last. This matches how we're
//   writing the zones in the ucdmesh. If there are no ghost zones then we
//   just return in the input array.
//
// Arguments:
//   arr : The data array to reorder.
//   gz  : The ghost zones.
//
// Returns:    A data array having the proper ordering.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue May 19 12:11:41 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static vtkDataArray *
ReorderUcdvarUsingGhostZones(vtkDataArray *arr, const unsigned char *gz)
{
    if(gz == 0)
    {
        arr->Register(NULL);
        return arr;
    }

    // This also converts to float
    vtkDataArray *retval = vtkFloatArray::New();
    retval->SetNumberOfComponents(arr->GetNumberOfComponents());
    retval->SetNumberOfTuples(arr->GetNumberOfTuples());
    vtkIdType id = 0, dest = 0;
    // Write the real values first
    for(id = 0; id < arr->GetNumberOfTuples(); ++id)
    {
         if(gz[id] == 0)
             retval->SetTuple(dest++, arr->GetTuple(id));
    }
    // Write the ghost values next
    for(id = 0; id < arr->GetNumberOfTuples(); ++id)
    {
         if(gz[id] > 0)
             retval->SetTuple(dest++, arr->GetTuple(id));
    }

    return retval;
}

// ****************************************************************************
//  Method: avtSiloWriter::WriteUcdvarsHelper
//
//  Purpose:
//      Writes out unstructured cell data variables.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 14:45:40 PST 2004
//    Added code to compute and store data extents
//
//    Mark C. Miller, Wed Jul 23 17:49:12 PDT 2008
//    Added code to handle point variables for point meshes
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 10:06:49 PDT 2009
//    Allow for subdirectories.
//
//    Brad Whitlock, Tue May 19 11:50:57 PDT 2009
//    Added support for reordering data using ghost zones.
//
// ****************************************************************************

void
avtSiloWriter::WriteUcdvarsHelper(DBfile *dbfile, vtkDataSetAttributes *ds, 
    bool isPointMesh, int centering, const unsigned char *gz)
{
    int i,j,k;

    for (i = 0 ; i < ds->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = ds->GetArray(i);

         //
         // find the associated extents vectors to update
         //
         vector<double> varMins;
         vector<double> varMaxs;
         std::map<string, vector<double> >::iterator minsMap;
         minsMap = dataMins.find(arr->GetName());
         if (minsMap != dataMins.end())
         {
             varMins = minsMap->second;
             varMaxs = dataMaxs[arr->GetName()];
         }

         if (strstr(arr->GetName(), "vtk") != NULL)
             continue;
         if (strstr(arr->GetName(), "avt") != NULL)
             continue;
         int nTuples = arr->GetNumberOfTuples();
         int ncomps = arr->GetNumberOfComponents();

         int nlevels = 0;
         string varName = BeginVar(dbfile, arr->GetName(), nlevels);
         string meshName = AbsoluteName(meshname, nlevels);

         vtkDataArray *arr2 = ReorderUcdvarUsingGhostZones(arr, gz);

         if (ncomps == 1)
         {
             // find min,max in this variable
             double dimMin = +DBL_MAX;
             double dimMax = -DBL_MAX;
             float *ptr    = (float *) arr->GetVoidPointer(0);
             for (k = 0 ; k < nTuples ; k++)
             {
                 if (ptr[k] < dimMin)
                     dimMin = ptr[k];
                 if (ptr[k] > dimMax)
                     dimMax = ptr[k];
             }
             varMins.push_back(dimMin);
             varMaxs.push_back(dimMax);

             if (isPointMesh && centering == DB_NODECENT)
                 DBPutPointvar1(dbfile, (char *) varName.c_str(),
                          (char *) meshName.c_str(),
                          (float *) arr2->GetVoidPointer(0),
                          nTuples, DB_FLOAT, optlist); 
             else
                 DBPutUcdvar1(dbfile, (char *) varName.c_str(),
                          (char *) meshName.c_str(),
                          (float *) arr2->GetVoidPointer(0), nTuples, NULL, 0,
                          DB_FLOAT, centering, optlist);
         }
         else
         {
             float **vars     = new float*[ncomps];
             float *ptr       = (float *) arr2->GetVoidPointer(0);
             char  **varnames = new char*[ncomps];
             for (j = 0 ; j < ncomps ; j++)
             {
                 double dimMin = +DBL_MAX;
                 double dimMax = -DBL_MAX;
                 vars[j] = new float[nTuples];
                 varnames[j] = new char[1024];
                 sprintf(varnames[j], "%s_comp%d", varName.c_str(), j);
                 for (k = 0 ; k < nTuples ; k++)
                 {
                     vars[j][k] = ptr[k*ncomps + j];
                     if (vars[j][k] < dimMin)
                         dimMin = vars[j][k];
                     if (vars[j][k] > dimMax)
                         dimMax = vars[j][k];
                 }
                 varMins.push_back(dimMin);
                 varMaxs.push_back(dimMax);
             }

             if (isPointMesh && centering == DB_NODECENT)
                 DBPutPointvar(dbfile, (char *) varName.c_str(),
                          (char *) meshName.c_str(),
                          ncomps, vars, nTuples, DB_FLOAT, optlist);
             else
                 DBPutUcdvar(dbfile, (char *) varName.c_str(),
                         (char *) meshName.c_str(),
                         ncomps, varnames, vars, nTuples, NULL, 0, DB_FLOAT,
                         centering, optlist);

             for (j = 0 ; j < ncomps ; j++)
             {
                  delete [] vars[j];
                  delete [] varnames[j];
             }
             delete [] vars;
             delete [] varnames;
         }
         arr2->Delete();

         EndVar(dbfile, nlevels);

         dataMins[arr->GetName()] = varMins;
         dataMaxs[arr->GetName()] = varMaxs;
    }
}

// ****************************************************************************
//  Method: avtSiloWriter::WriteUcdvars
//
//  Purpose:
//      Writes out unstructured cell data variables.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Mar 6 14:32:43 PST 2009
//    Use helper functions.
//
//    Brad Whitlock, Tue May 19 11:47:37 PDT 2009
//    Add support for ghost zones.
//
// ****************************************************************************

void
avtSiloWriter::WriteUcdvars(DBfile *dbfile, vtkPointData *pd,
    vtkCellData *cd, bool isPointMesh, const unsigned char *gz)
{
    WriteUcdvarsHelper(dbfile, pd, isPointMesh, DB_NODECENT, 0);
    WriteUcdvarsHelper(dbfile, cd, isPointMesh, DB_ZONECENT, gz);
}

// ****************************************************************************
//  Method: avtSiloWriter::WriteQuadvarsHelper
//
//  Purpose:
//      Writes out quadvars.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 14:39:51 PST 2009
//    Allow for subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::WriteQuadvarsHelper(DBfile *dbfile, vtkDataSetAttributes *ds,
   int ndims, int *dims, int centering)
{
    int i,j,k;

    for (i = 0 ; i < ds->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = ds->GetArray(i);

         //
         // find the associated extents vectors to update
         //
         vector<double> varMins;
         vector<double> varMaxs;
         std::map<string, vector<double> >::iterator minsMap;
         minsMap = dataMins.find(arr->GetName());
         if (minsMap != dataMins.end())
         {
             varMins = minsMap->second;
             varMaxs = dataMaxs[arr->GetName()];
         }

         if (strstr(arr->GetName(), "vtk") != NULL)
             continue;
         if (strstr(arr->GetName(), "avt") != NULL)
             continue;
         int ncomps = arr->GetNumberOfComponents();
         int nTuples = arr->GetNumberOfTuples();

         int nlevels = 0;
         string varName = BeginVar(dbfile, arr->GetName(), nlevels);
         string meshName = AbsoluteName(meshname, nlevels);

         if (ncomps == 1)
         {
             // find min,max in this variable
             double dimMin = +DBL_MAX;
             double dimMax = -DBL_MAX;
             float *ptr    = (float *) arr->GetVoidPointer(0);
             for (k = 0 ; k < nTuples ; k++)
             {
                 if (ptr[k] < dimMin)
                     dimMin = ptr[k];
                 if (ptr[k] > dimMax)
                     dimMax = ptr[k];
             }
             varMins.push_back(dimMin);
             varMaxs.push_back(dimMax);

             DBPutQuadvar1(dbfile, (char *) varName.c_str(),
                           (char *) meshName.c_str(),
                           (float *) ptr, dims, ndims, NULL,
                           0, DB_FLOAT, centering, optlist);
         }
         else
         {
             float **vars     = new float*[ncomps];
             float *ptr       = (float *) arr->GetVoidPointer(0);
             char  **varnames = new char*[ncomps];
             for (j = 0 ; j < ncomps ; j++)
             {
                 double dimMin = +DBL_MAX;
                 double dimMax = -DBL_MAX;
                 vars[j] = new float[nTuples];
                 varnames[j] = new char[1024];
                 sprintf(varnames[j], "%s_comp%d", arr->GetName(), j);
                 for (k = 0 ; k < nTuples ; k++)
                 {
                     vars[j][k] = ptr[k*ncomps + j];
                     if (vars[j][k] < dimMin)
                         dimMin = vars[j][k];
                     if (vars[j][k] > dimMax)
                         dimMax = vars[j][k];
                 }
                 varMins.push_back(dimMin);
                 varMaxs.push_back(dimMax);
             }

             DBPutQuadvar(dbfile, (char *) varName.c_str(),
                          (char *) meshName.c_str(),
                          ncomps, varnames, vars, dims, ndims, NULL, 0, 
                          DB_FLOAT, centering, optlist);
             for (j = 0 ; j < ncomps ; j++)
             {
                  delete [] vars[j];
                  delete [] varnames[j];
             }
             delete [] vars;
             delete [] varnames;
         }

         EndVar(dbfile, nlevels);

         dataMins[arr->GetName()] = varMins;
         dataMaxs[arr->GetName()] = varMaxs;
    }
}

// ****************************************************************************
//  Method: avtSiloWriter::WriteQuadvars
//
//  Purpose:
//      Writes out quadvars.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Brad Whitlock, Fri Mar 6 14:39:51 PST 2009
//    Use helper functions.
//
// ****************************************************************************

void
avtSiloWriter::WriteQuadvars(DBfile *dbfile, vtkPointData *pd,
                                vtkCellData *cd, int ndims, int *dims)
{
    int zdims[3];
    zdims[0] = (ndims > 0 ? dims[0]-1 : 0);
    zdims[1] = (ndims > 1 ? dims[1]-1 : 0);
    zdims[2] = (ndims > 2 ? dims[2]-1 : 0);

    WriteQuadvarsHelper(dbfile, pd, ndims, dims, DB_NODECENT);
    WriteQuadvarsHelper(dbfile, cd, ndims, zdims, DB_ZONECENT);
}

// ****************************************************************************
//  Method: avtSiloWriter::WriteMaterials
//
//  Purpose:
//      Writes out the materials construct.  This routine will "do the right
//      thing" based on whether or not MIR has already occurred.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
//
//    Brad Whitlock, Fri Mar 6 14:43:31 PST 2009
//    Allow for subdirectories.
//
// ****************************************************************************

void
avtSiloWriter::WriteMaterials(DBfile *dbfile, vtkCellData *cd, int chunk)
{
    int   i;

    if (!hasMaterialsInProblem)
        return;

    if (mustGetMaterialsAdditionally)
    {
        avtMaterial *mat = GetInput()->GetOriginatingSource()
                                           ->GetMetaData()->GetMaterial(chunk);
        if (mat == NULL)
        {
            debug1 << "Not able to get requested material" << endl;
            return;
        }

        //
        // The AVT material structure closely matches that of the Silo
        // materials.  Just make a few small conversions and write it out.
        //
        int nmats = mat->GetNMaterials();
        int *matnos = new int[nmats];
        for (i = 0 ; i < nmats ; i++)
            matnos[i] = i;
        int nzones[1];
        nzones[0] = mat->GetNZones();

        int nlevels = 0;
        string matName = BeginVar(dbfile, matname, nlevels);
        string meshName = AbsoluteName(meshname, nlevels);

        DBPutMaterial(dbfile, (char *) matName.c_str(),
                      (char *) meshName.c_str(), nmats, matnos,
                       (int *) mat->GetMatlist(), nzones, 1, 
                       (int *) mat->GetMixNext(), (int *) mat->GetMixMat(),
                       (int *) mat->GetMixZone(), (float *) mat->GetMixVF(),
                       mat->GetMixlen(), DB_FLOAT, optlist);
        delete [] matnos;

        EndVar(dbfile, nlevels);
    }
    else
    {
        vtkDataArray *arr = cd->GetArray("avtSubsets");
        if (arr == NULL)
        {
            debug1 << "Subsets array not available" << endl;
            return;
        }
        if (arr->GetDataType() != VTK_INT)
        {
            debug1 << "Subsets array not of right type" << endl;
            return;
        }

        //
        // We are going to have to calculate each of the structures we want
        // to write out.  Start by determining which material numbers are used
        // and which are not.
        //
        vtkIntArray *ia = (vtkIntArray *) arr;
        vector<bool> matlookup;
        int nzones = ia->GetNumberOfTuples();
        for (i = 0 ; i < nzones ; i++)
        {
            int matno = ia->GetValue(i);
            while (matlookup.size() <= matno)
            {
                matlookup.push_back(false);
            }
            matlookup[matno] = true;
        }

        //
        // Now determine the number of materials.
        //
        int nmats = 0;
        for (i = 0 ; i < matlookup.size() ; i++)
            if (matlookup[i])
                nmats++;

        //
        // Create the matnos array.
        //
        int *matnos = new int[nmats];
        int dummy = 0;
        for (i = 0 ; i < matlookup.size() ; i++)
            if (matlookup[i])
                matnos[dummy++] = i;

        //
        // Do the actual write.  This isn't too bad since we have all
        // clean zones.
        //
        int nlevels = 0;
        string matName = BeginVar(dbfile, matname, nlevels);
        string meshName = AbsoluteName(meshname, nlevels);

        DBPutMaterial(dbfile, (char *) matName.c_str(),
                      (char *) meshName.c_str(), nmats, matnos,
                       ia->GetPointer(0), &nzones, 1, NULL, NULL, NULL,
                       NULL, 0, DB_FLOAT, optlist);
        delete [] matnos;

        EndVar(dbfile, nlevels);
    }
}

// ****************************************************************************
// Method: avtSiloWriter::WriteExpressions
//
// Purpose: 
//   This method writes out the expressions that originally came from the
//   database to Silo.
//
// Arguments:
//   dbfile : The Silo file that we're outputting.
//
// Returns:    
//
// Note:       This is primarily meant to export expressions for the master file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 11 11:26:22 PDT 2009
//
// Modifications:
//   
//   Hank Childs, Mon May 25 11:14:23 PDT 2009
//   Add support for old Silo versions.
//
// ****************************************************************************

void
avtSiloWriter::WriteExpressions(DBfile *dbfile)
{
    const ExpressionList &expr = headerDbMd->GetExprList();
    int i, ecount = 0;
    for(i = 0; i < expr.GetNumExpressions(); ++i)
        ecount += (expr.GetExpressions(i).GetFromDB() ? 1 : 0);
    if(ecount > 0)
    {
        // Pack up the definitions into arrays that we can write out.
        char **exprNames = new char*[ecount];
        int   *exprTypes = new int[ecount];
        char **exprDefs  = new char*[ecount];
        ecount = 0;
        for(i = 0; i < expr.GetNumExpressions(); ++i)
        {
            const Expression &e = expr.GetExpressions(i);
            if(e.GetFromDB())
            {
                exprNames[ecount] = new char[e.GetName().size()+1];
                strcpy(exprNames[ecount], e.GetName().c_str());

                int vartype = 0;
                switch(e.GetType())
                {
                case Expression::ScalarMeshVar: vartype = DB_VARTYPE_SCALAR;   break;
                case Expression::VectorMeshVar: vartype = DB_VARTYPE_VECTOR;   break;
                case Expression::TensorMeshVar: vartype = DB_VARTYPE_TENSOR;   break;
                case Expression::ArrayMeshVar:  vartype = DB_VARTYPE_ARRAY;    break;
                case Expression::Material:      vartype = DB_VARTYPE_MATERIAL; break;
                case Expression::Species :      vartype = DB_VARTYPE_SPECIES;  break;
                default:                        vartype = DB_VARTYPE_SCALAR;   break;
                }
                exprTypes[ecount] = vartype;

                exprDefs[ecount] = new char[e.GetDefinition().size()+1];
                strcpy(exprDefs[ecount], e.GetDefinition().c_str());

                ++ecount;
            }
        }

        // Write the definitions
        // 4.6.1 chosen arbitrarily. 
#ifdef SILO_VERSION_GE
#if SILO_VERSION_GE(4,6,1)
        DBPutDefvars(dbfile, "expressions", ecount, exprNames, exprTypes, exprDefs, NULL);
#else
        DBPutDefvars(dbfile, "expressions", ecount, (const char **) exprNames, exprTypes, (const char **) exprDefs, NULL);
#endif
#else
        DBPutDefvars(dbfile, "expressions", ecount, (const char **) exprNames, exprTypes, (const char **) exprDefs, NULL);
#endif

        // Clean up
        for(i = 0; i < ecount; ++i)
        {
            delete [] exprNames[i];
            delete [] exprDefs[i];
        }
        delete [] exprNames;
        delete [] exprTypes;
        delete [] exprDefs;
    }
}

// ****************************************************************************
//  Method: avtSiloWriter::VTKZoneTypeToSiloZoneType
//
//  Purpose:
//      Converts a VTK cell type to the proper Silo zone type.
//
//  Arguments:
//      vtk_zonetype     Input VTK zone type.
//
//  Returns:     Silo zone type
//
//  Programmer: Cyrus Harrison
//  Creation:   February 26, 2007
//
// ****************************************************************************

int
avtSiloWriter::VTKZoneTypeToSiloZoneType(int vtk_zonetype)
{
    int  silo_zonetype = -1;

    switch (vtk_zonetype)
    {
      case VTK_POLYGON:
        silo_zonetype = DB_ZONETYPE_POLYGON;
        break;
      case VTK_TRIANGLE:
        silo_zonetype = DB_ZONETYPE_TRIANGLE;
        break;
      case VTK_PIXEL:
      case VTK_QUAD:
        silo_zonetype = DB_ZONETYPE_QUAD;
        break;
      case VTK_TETRA:
        silo_zonetype = DB_ZONETYPE_TET;
        break;
      case VTK_PYRAMID:
        silo_zonetype = DB_ZONETYPE_PYRAMID;
        break;
      case VTK_WEDGE:
        silo_zonetype = DB_ZONETYPE_PRISM;
        break;
      case VTK_VOXEL:
      case VTK_HEXAHEDRON:
        silo_zonetype = DB_ZONETYPE_HEX;
        break;
      case VTK_LINE:
        silo_zonetype = DB_ZONETYPE_BEAM;
        break;
    }

    return silo_zonetype;
}
