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

#include <float.h>
#include <ctype.h>

#include <vector>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtMetaData.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>

#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

#include <silo.h>


using std::map;
using std::string;
using std::vector;


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
        if (isalnum(vn[km][i]) || vn[km][i] == '_')
            continue;
        vn[km][i] = '_';
    }

    return vn[km].c_str();
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
// ****************************************************************************

avtSiloWriter::avtSiloWriter(DBOptionsAttributes *dbopts)
{
    headerDbMd = 0;
    optlist = 0;
    nblocks = 0;
    driver = DB_PDB;
    singleFile = false;

    for (int i = 0; dbopts != 0 && i < dbopts->GetNumberOfOptions(); ++i)
    {
        if (dbopts->GetName(i) == "Driver")
        {
            switch (dbopts->GetEnum("Driver"))
            {
                case 0: driver = DB_PDB; break;
	        case 1: driver = DB_HDF5; break;
            }
        }
        else if (dbopts->GetName(i) == "Single File")
        {
            singleFile = dbopts->GetBool("Single File");
#if defined(PARALLEL)
            if (singleFile)
            {
                debug1 << "Cannot guarentee single file option from parallel engine. Ignoring." << endl;
                singleFile = false;
            }
#endif
        }
        else
            debug1 << "Ignoring unknown option \"" << dbopts->GetName(i) << "\"" << endl;
    }

    meshtype = AVT_UNKNOWN_MESH;

}


// ****************************************************************************
//  Method: avtSiloWriter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
// ****************************************************************************

avtSiloWriter::~avtSiloWriter()
{
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
// ****************************************************************************

void
avtSiloWriter::ConstructMultimesh(DBfile *dbfile, const avtMeshMetaData *mmd)
{
    int   i,j,k;

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
            sprintf(tmp, "domain_%d/%s", i, VN(mmd->name));
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
    DBPutMultimesh(dbfile, (char *) VN(mmd->name),
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
// ****************************************************************************

void
avtSiloWriter::ConstructMultivar(DBfile *dbfile, const string &sname,
                                 const avtMeshMetaData *mmd)
{
    int   i,j,k;

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
            sprintf(tmp, "domain_%d/%s", i, VN(sname));
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
    if (extents != 0)
    {
#if defined(DBOPT_EXTENTS_SIZE) && !defined(PARALLEL)
        int extsize = ncomps * 2;
        DBAddOption(tmpOptlist, DBOPT_EXTENTS_SIZE, &extsize);
        DBAddOption(tmpOptlist, DBOPT_EXTENTS, extents);
#endif
    }

    //
    // Write the actual header information.
    //
    DBPutMultivar(dbfile, (char *) VN(sname),
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
// ****************************************************************************

void
avtSiloWriter::ConstructMultimat(DBfile *dbfile, const string &mname,
                                 const avtMeshMetaData *mmd)
{
    int   i;

    //
    // Construct the names for each mat
    //
    char **mat_names = new char*[nblocks];
    for (i = 0 ; i < nblocks ; i++)
    {
        char tmp[1024];
        if (singleFile)
            sprintf(tmp, "domain_%d/%s", i, VN(mname));
        else
            sprintf(tmp, "%s.%d.silo:/%s", stem.c_str(), i, VN(mname));
        mat_names[i] = new char[strlen(tmp)+1];
        strcpy(mat_names[i], tmp);
    }

    //
    // Write the actual header information.
    //
    DBPutMultimat(dbfile, (char *) VN(mname), nblocks, mat_names, optlist);

    //
    // Clean up memory.
    //
    for (i = 0 ; i < nblocks ; i++)
    {
        delete [] mat_names[i];
    }
    delete [] mat_names;
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

#ifdef E_CHECKSUM
    int oldEnable = DBSetEnableChecksums(0);
#endif

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

        if (singleFile)
        {
            char dirname[32];
            sprintf(dirname, "domain_%d", chunk);
            DBMkDir(dbfile, dirname);
            DBSetDir(dbfile, dirname);
        }
    }

#ifdef E_CHECKSUM
    DBSetEnableChecksums(oldEnable);
#endif
    
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
        debug5 << "avtSiloWriter: proc " << procid << " writting silo root"
               << "file" << endl;
        char filename[1024];
        string fname = dir + stem;
        sprintf(filename, "%s.silo", fname.c_str());
#ifdef E_CHECKSUM
        int oldEnable = DBSetEnableChecksums(0);
#endif
        DBfile *dbfile;
        if (singleFile)
            dbfile = DBOpen(filename, DB_UNKNOWN, DB_APPEND);
        else
            dbfile = DBCreate(filename, 0, DB_LOCAL, 
                         "Silo file written by VisIt", driver);
#ifdef E_CHECKSUM
        DBSetEnableChecksums(oldEnable);
#endif

        ConstructMultimesh(dbfile, mmd);
        for (i = 0 ; i < headerScalars.size() ; i++)
            ConstructMultivar(dbfile, headerScalars[i], mmd);
        for (i = 0 ; i < headerVectors.size() ; i++)
            ConstructMultivar(dbfile, headerVectors[i], mmd);
        for (i = 0 ; i < headerMaterials.size() ; i++)
            ConstructMultimat(dbfile, headerMaterials[i], mmd);

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

    //
    // Put the zone list into a digestable form for Silo.
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
    
    if (npointcells == nzones && npointcells == npts)
    {
        DBPutPointmesh(dbfile, VN(meshname), dim, coords,
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

        DBPutZonelist2(dbfile, "zonelist", nzones, dim, zl, lzl, 
                       0, 0, 0, st, ss, sc,nshapes, NULL);
    
        //
        // Now write the actual mesh.
        //
        DBPutUcdmesh(dbfile, (char *) VN(meshname), dim, NULL, coords, npts,
                     nzones, "zonelist", NULL, DB_FLOAT, optlist);
    }

    WriteUcdvars(dbfile, ug->GetPointData(), ug->GetCellData(), npointcells == nzones);
    WriteMaterials(dbfile, ug->GetCellData(), chunk);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];
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
    DBPutQuadmesh(dbfile, (char *) VN(meshname), NULL, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, optlist);

    WriteQuadvars(dbfile, sg->GetPointData(), sg->GetCellData(),ndims,dims);
    WriteMaterials(dbfile, sg->GetCellData(), chunk);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];
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
    DBPutQuadmesh(dbfile, (char *) VN(meshname), NULL, coords, dims, ndims,
                  DB_FLOAT, DB_COLLINEAR, optlist);

    WriteQuadvars(dbfile, rg->GetPointData(), rg->GetCellData(),ndims,dims);
    WriteMaterials(dbfile, rg->GetCellData(), chunk);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];
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

    if (npointcells == nzones && npointcells == npts)
    {
        DBPutPointmesh(dbfile, (char *) VN(meshname), ndims, coords,
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
        DBPutUcdmesh(dbfile, (char *) VN(meshname), ndims, NULL, coords, npts,
                     nzones, "zonelist", NULL, DB_FLOAT, optlist);
    }

    WriteUcdvars(dbfile, pd->GetPointData(), pd->GetCellData(),
        npointcells == nzones);
    WriteMaterials(dbfile, pd->GetCellData(), chunk);

    //
    // Free up memory.
    //
    if (coords[0] != NULL)
        delete [] coords[0];
    if (coords[1] != NULL)
        delete [] coords[1];
    if (coords[2] != NULL)
        delete [] coords[2];
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
//    Mark C. Miller, Tue Mar  9 14:45:40 PST 2004
//    Added code to compute and store data extents
//
//    Mark C. Miller, Wed Jul 23 17:49:12 PDT 2008
//    Added code to handle point variables for point meshes
//
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
// ****************************************************************************

void
avtSiloWriter::WriteUcdvars(DBfile *dbfile, vtkPointData *pd,
                               vtkCellData *cd, bool isPointMesh)
{
    int   i, j, k;

    //
    // Write out the nodal variables.
    //
    for (i = 0 ; i < pd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = pd->GetArray(i);

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
         int npts = arr->GetNumberOfTuples();
         int ncomps = arr->GetNumberOfComponents();
         if (ncomps == 1)
         {
             // find min,max in this variable
             double dimMin = +DBL_MAX;
             double dimMax = -DBL_MAX;
             float *ptr    = (float *) arr->GetVoidPointer(0);
             for (k = 0 ; k < npts ; k++)
             {
                 if (ptr[k] < dimMin)
                     dimMin = ptr[k];
                 if (ptr[k] > dimMax)
                     dimMax = ptr[k];
             }
             varMins.push_back(dimMin);
             varMaxs.push_back(dimMax);

             if (isPointMesh)
                 DBPutPointvar1(dbfile, (char *) VN(arr->GetName()),
                          (char *) VN(meshname),
                          (float *) arr->GetVoidPointer(0),
                          npts, DB_FLOAT, optlist); 
             else
                 DBPutUcdvar1(dbfile, (char *) VN(arr->GetName()),
                          (char *) VN(meshname),
                          (float *) arr->GetVoidPointer(0), npts, NULL, 0,
                          DB_FLOAT, DB_NODECENT, optlist);
         }
         else
         {
             float **vars     = new float*[ncomps];
             float *ptr       = (float *) arr->GetVoidPointer(0);
             char  **varnames = new char*[ncomps];
             vector<double> varMins;
             vector<double> varMaxs;
             for (j = 0 ; j < ncomps ; j++)
             {
                 double dimMin = +DBL_MAX;
                 double dimMax = -DBL_MAX;
                 vars[j] = new float[npts];
                 varnames[j] = new char[1024];
                 sprintf(varnames[j], "%s_comp%d", arr->GetName(), j);
                 for (k = 0 ; k < npts ; k++)
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

             if (isPointMesh)
                 DBPutPointvar(dbfile, (char *) VN(arr->GetName()),
                          (char *) VN(meshname),
                          ncomps, vars, npts, DB_FLOAT, optlist);
             else
                 DBPutUcdvar(dbfile, (char *) VN(arr->GetName()),
                         (char *) VN(meshname),
                         ncomps, varnames, vars, npts, NULL, 0, DB_FLOAT,
                         DB_NODECENT, optlist);

             for (j = 0 ; j < ncomps ; j++)
             {
                  delete [] vars[j];
                  delete [] varnames[j];
             }
             delete [] vars;
             delete [] varnames;
         }

         dataMins[arr->GetName()] = varMins;
         dataMaxs[arr->GetName()] = varMaxs;
    }

    //
    // Write out the zonal variables.
    //
    for (i = 0 ; i < cd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = cd->GetArray(i);

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
         int nzones = arr->GetNumberOfTuples();
         if (ncomps == 1)
         {

             // find min,max in this variable
             double dimMin = +DBL_MAX;
             double dimMax = -DBL_MAX;
             float *ptr    = (float *) arr->GetVoidPointer(0);
             for (k = 0 ; k < nzones ; k++)
             {
                 if (ptr[k] < dimMin)
                     dimMin = ptr[k];
                 if (ptr[k] > dimMax)
                     dimMax = ptr[k];
             }
             varMins.push_back(dimMin);
             varMaxs.push_back(dimMax);

             DBPutUcdvar1(dbfile, (char *) VN(arr->GetName()),
                          (char *) VN(meshname),
                          (float *) arr->GetVoidPointer(0), nzones, NULL, 0,
                          DB_FLOAT, DB_ZONECENT, optlist);
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
                 vars[j] = new float[nzones];
                 varnames[j] = new char[1024];
                 sprintf(varnames[j], "%s_comp%d", arr->GetName(), j);
                 for (k = 0 ; k < nzones ; k++)
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

             DBPutUcdvar(dbfile, (char *) VN(arr->GetName()),
                         (char *) VN(meshname),
                         ncomps, varnames, vars, nzones, NULL, 0, DB_FLOAT,
                         DB_ZONECENT, optlist);
             for (j = 0 ; j < ncomps ; j++)
             {
                  delete [] vars[j];
                  delete [] varnames[j];
             }
             delete [] vars;
             delete [] varnames;
         }

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
//    Mark C. Miller, Tue Aug 26 14:29:46 PDT 2008
//    Made it construct valid variable name(s) for silo objects.
// ****************************************************************************

void
avtSiloWriter::WriteQuadvars(DBfile *dbfile, vtkPointData *pd,
                                vtkCellData *cd, int ndims, int *dims)
{
    int   i, j, k;

    int zdims[3];
    zdims[0] = (ndims > 0 ? dims[0]-1 : 0);
    zdims[1] = (ndims > 1 ? dims[1]-1 : 0);
    zdims[2] = (ndims > 2 ? dims[2]-1 : 0);

    //
    // Write out the nodal variables.
    //
    for (i = 0 ; i < pd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = pd->GetArray(i);

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
         int npts = arr->GetNumberOfTuples();
         if (ncomps == 1)
         {
             // find min,max in this variable
             double dimMin = +DBL_MAX;
             double dimMax = -DBL_MAX;
             float *ptr    = (float *) arr->GetVoidPointer(0);
             for (k = 0 ; k < npts ; k++)
             {
                 if (ptr[k] < dimMin)
                     dimMin = ptr[k];
                 if (ptr[k] > dimMax)
                     dimMax = ptr[k];
             }
             varMins.push_back(dimMin);
             varMaxs.push_back(dimMax);

             DBPutQuadvar1(dbfile, (char *) VN(arr->GetName()),
                           (char *) VN(meshname),
                           (float *) ptr, dims, ndims, NULL,
                           0, DB_FLOAT, DB_NODECENT, optlist);
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
                 vars[j] = new float[npts];
                 varnames[j] = new char[1024];
                 sprintf(varnames[j], "%s_comp%d", arr->GetName(), j);
                 for (k = 0 ; k < npts ; k++)
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

             DBPutQuadvar(dbfile, (char *) VN(arr->GetName()),
                          (char *) VN(meshname),
                          ncomps, varnames, vars, dims, ndims, NULL, 0, 
                          DB_FLOAT, DB_NODECENT, optlist);
             for (j = 0 ; j < ncomps ; j++)
             {
                  delete [] vars[j];
                  delete [] varnames[j];
             }
             delete [] vars;
             delete [] varnames;
         }

         dataMins[arr->GetName()] = varMins;
         dataMaxs[arr->GetName()] = varMaxs;
    }

    //
    // Write out the zonal variables.
    //
    for (i = 0 ; i < cd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = cd->GetArray(i);

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
         int nzones = arr->GetNumberOfTuples();
         if (ncomps == 1)
         {
             // find min,max in this variable
             double dimMin = +DBL_MAX;
             double dimMax = -DBL_MAX;
             float *ptr    = (float *) arr->GetVoidPointer(0);
             for (k = 0 ; k < nzones ; k++)
             {
                 if (ptr[k] < dimMin)
                     dimMin = ptr[k];
                 if (ptr[k] > dimMax)
                     dimMax = ptr[k];
             }
             varMins.push_back(dimMin);
             varMaxs.push_back(dimMax);

             DBPutQuadvar1(dbfile, (char *) VN(arr->GetName()),
                           (char *) VN(meshname),
                           (float *) arr->GetVoidPointer(0), zdims, ndims,
                           NULL, 0, DB_FLOAT, DB_ZONECENT, optlist);
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
                 vars[j] = new float[nzones];
                 varnames[j] = new char[1024];
                 sprintf(varnames[j], "%s_comp%d", arr->GetName(), j);
                 for (k = 0 ; k < nzones ; k++)
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

             DBPutQuadvar(dbfile, (char *) VN(arr->GetName()),
                          (char *) VN(meshname),
                          ncomps, varnames, vars, zdims, ndims, NULL, 0, 
                          DB_FLOAT, DB_ZONECENT, optlist);
             for (j = 0 ; j < ncomps ; j++)
             {
                  delete [] vars[j];
                  delete [] varnames[j];
             }
             delete [] vars;
             delete [] varnames;
         }

         dataMins[arr->GetName()] = varMins;
         dataMaxs[arr->GetName()] = varMaxs;
    }
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
        DBPutMaterial(dbfile, (char *) VN(matname.c_str()),
                      (char *) VN(meshname), nmats, matnos,
                       (int *) mat->GetMatlist(), nzones, 1, 
                       (int *) mat->GetMixNext(), (int *) mat->GetMixMat(),
                       (int *) mat->GetMixZone(), (float *) mat->GetMixVF(),
                       mat->GetMixlen(), DB_FLOAT, optlist);
        delete [] matnos;
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
        DBPutMaterial(dbfile, (char *) VN(matname.c_str()),
                      (char *) VN(meshname), nmats, matnos,
                       ia->GetPointer(0), &nzones, 1, NULL, NULL, NULL,
                       NULL, 0, DB_FLOAT, optlist);
        delete [] matnos;
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
