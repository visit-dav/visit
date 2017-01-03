/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                            avtPFLOTRANFileFormat.C                           //
// ************************************************************************* //

#include <avtPFLOTRANFileFormat.h>

#include <DebugStream.h>
#include <snprintf.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtDatabaseMetaData.h>
#include <avtDatabase.h>
#include <avtMaterial.h>

#include <InvalidDBTypeException.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <visit-hdf5.h>

using std::map;
using std::string;
using std::vector;

static string coordNames[3] = { "X [m]", "Y [m]", "Z [m]" };
static string vecNames[3] = { "X-", "Y-", "Z-" };

// ****************************************************************************
//  Method: avtPFLOTRANFileFormat constructor
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  3 12:26:26 EDT 2009
//    All variables are supposed to be cell-centered.
//    Add support for old-style PFLOTRAN files where the coordinate
//    arrays represented cell centers.
//
//    Jeremy Meredith, Wed Mar 30 12:57:00 EDT 2011
//    Initialize various HDF5 ids to -1.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.
//
// ****************************************************************************

avtPFLOTRANFileFormat::avtPFLOTRANFileFormat(const char *fname):
    avtMTMDFileFormat(fname)
{
    unstructured = false;

    fileID = -1;
    dimID[0] = -1;
    dimID[1] = -1;
    dimID[2] = -1;
    cellsID = -1;
    vertsID = -1;

    filename = strdup(fname);
    opened = false;
    nTime = 0;

    // a bit of a hack to fix up old files with cell-centered coordinates
    oldFileNeedingCoordFixup = false;

    // Turn off HDF5 error messages to the terminal.
    H5Eset_auto(NULL, NULL);
}

// ****************************************************************************
//  Method: avtPFLOTRANFileFormat destructor
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 30 12:30:09 EDT 2011
//    Close the data sets we opened earlier.  Also, we can now check for an
//    error during H5Fclose (due to file access property setting when opening
//    the file), and report it if we encountered it.
//
//    Jeremy Meredith, Wed Jun 15 14:55:51 EDT 2011
//    Move file-closing code here from FreeUpResources -- that's called
//    when changing time steps; we don't want to close the file then.
//    Also, don't make the check report an error; that's too severe.
//    Just print to cerr.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.
//
// ****************************************************************************

avtPFLOTRANFileFormat::~avtPFLOTRANFileFormat()
{
    free(filename);
    filename = NULL;

    if (dimID[0] > 0)
        H5Dclose(dimID[0]);
    if (dimID[1] > 0)
        H5Dclose(dimID[1]);
    if (dimID[2] > 0)
        H5Dclose(dimID[2]);
    if (cellsID > 0)
        H5Dclose(cellsID);
    if (vertsID > 0)
        H5Dclose(vertsID);
    dimID[0] = -1;
    dimID[1] = -1;
    dimID[2] = -1;
    cellsID = -1;
    vertsID = -1;

    if (fileID > 0)
    {
        herr_t err = H5Fclose(fileID);
        if (err < 0)
        {
            cerr <<    "avtPFLOTRANFileFormat::~avtPFLOTRANFileFormat: "
                       "Couldn't close the file properly; this is likely due "
                       "to not closing all open objects in the file.  Please "
                       "report this to a VisIt developer.\n";
        }
    }
    fileID = -1;
    opened = false;

}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
// ****************************************************************************

int
avtPFLOTRANFileFormat::GetNTimesteps(void)
{
    LoadFile();

    return nTime;
}

// ****************************************************************************
//  Method: avtPFLOTRANFileFormat::LoadFile
//
//  Purpose:
//      Loads the basics of the file into memory.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  1 12:43:11 EDT 2008
//    Added support for automatic parallel decomposition and parallel I/O
//    via hyperslab reading.
//
//    Eric Brugger, Fri Aug  8 13:53:01 PDT 2008
//    Added an explicit conversion from a char* to a string to make the
//    MIPSpro compiler happy.
//
//    Jeremy Meredith, Thu Jan  7 15:36:19 EST 2010
//    Close all open ids when returning an exception.
//
//    Jeremy Meredith, Wed Mar 30 12:33:56 EDT 2011
//    More ids left to close.  Also, open the file in a way that is more
//    strict about detecting unclosed files/ids.
//
//    Jeremy Meredith, Wed Jun 15 15:56:44 EDT 2011
//    Missed an ID to close.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.
//
// ****************************************************************************

void
avtPFLOTRANFileFormat::LoadFile(void)
{
    if (opened == true)
        return;

    // Initialize HDF5.
    H5open();

    // We want it to be aggressive about closing the file when we tell it to.
    hid_t fileAccessPropListID = H5Pcreate(H5P_FILE_ACCESS);
    if (fileAccessPropListID < 0)
    {
        EXCEPTION1(ImproperUseException, "Couldn't H5Pcreate");
    }
    herr_t err = H5Pset_fclose_degree(fileAccessPropListID, H5F_CLOSE_SEMI);
    if (err < 0)
    {
        EXCEPTION1(ImproperUseException, "Couldn't set file close access");
    }

    // Grab the file and begin.
    fileID = H5Fopen(filename, H5F_ACC_RDONLY, fileAccessPropListID);
    H5Pclose(fileAccessPropListID);
    if (fileID < 0)
    {
        debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open <" << filename << ">" << endl;
        char error[1024];
        SNPRINTF(error, 1024, "Cannot be a PFLOTRAN file (%s) since it is not even an HDF5 file:",filename);
        EXCEPTION1(InvalidDBTypeException, error);
    }

    // Check more mesh structure
    hid_t coordsGID = H5Gopen(fileID, "Coordinates");
    if (coordsGID >= 0)
    {
        // We have coordinates; it's structured.
        // Pull out the mesh dimensions while we're at it.
        for (int dim=0;dim<3;dim++)
        {
            dimID[dim] = H5Dopen(coordsGID, coordNames[dim].c_str());
            if (dimID[dim] < 0)
            {
                H5Gclose(coordsGID);
                H5Fclose(fileID);
                debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open the " << coordNames[dim] << "dataset in file " << filename << endl;
                EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have valid coordinates data.");
            }
            hid_t dimSpaceID = H5Dget_space(dimID[dim]);
            if (dimSpaceID < 0)
            {
                H5Gclose(coordsGID);
                H5Fclose(fileID);
                debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not get the space information for the " << coordNames[dim] << " coordinate in file " << filename << endl;
                EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have valid coordinates data.");
            }
            int ndims = H5Sget_simple_extent_ndims(dimSpaceID);
            if (ndims != 1)
            {
                H5Gclose(coordsGID);
                H5Fclose(fileID);
                debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "The " << coordNames[dim] << " coordinate is not one dimensional" << endl;
                EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since some coordinate data is not one dimensional.");
            }
            hsize_t dims, maxdims;
            H5Sget_simple_extent_dims(dimSpaceID, &dims, &maxdims);

            globalDims[dim] = dims;
        }
        H5Gclose(coordsGID);
    }
    else
    {
        // No coordinates.  could it be an unstructured grid?
        hid_t domainGID = H5Gopen(fileID, "Domain");
        if (domainGID < 0)
        {
            H5Fclose(fileID);
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open the Coordinates or Domain group in file " << filename << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have a top-level Coordinates or Domain group.");
        }

        cellsID = H5Dopen(domainGID, "Cells");
        vertsID = H5Dopen(domainGID, "Vertices");
        if (cellsID < 0 || vertsID < 0)
        {
            if (cellsID >= 0)
                H5Dclose(cellsID);
            if (vertsID >= 0)
                H5Dclose(vertsID);
            H5Gclose(domainGID);
            H5Fclose(fileID);
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open the Cells/Vertices data set in Domain group, file " << filename << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have a Vertices and Cells data set in its Domain group.");
        }

        hid_t cellsSpaceID = H5Dget_space(cellsID);
        int cellsNdims = H5Sget_simple_extent_ndims(cellsSpaceID);
        hid_t vertsSpaceID = H5Dget_space(vertsID);
        int vertsNdims = H5Sget_simple_extent_ndims(vertsSpaceID);
        if (cellsNdims != 2 || vertsNdims != 2)
        {
            H5Dclose(cellsID);
            H5Dclose(vertsID);
            H5Gclose(domainGID);
            H5Fclose(fileID);
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Expected 2-dimensional Cells and Vertices arrays in Domain group in file " << filename << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since Vertices and Cells data sets in its Domain group are not 2-dimensional.");
        }

        hsize_t dims[2], maxdims[2];

        H5Sget_simple_extent_dims(cellsSpaceID, dims, maxdims);
        ucd_ncells = dims[0];
        ucd_cellstride = dims[1];

        H5Sget_simple_extent_dims(vertsSpaceID, dims, maxdims);
        ucd_nverts = dims[0];
        ucd_vertdim = dims[1];
        if (ucd_vertdim != 3)
        {
            H5Dclose(cellsID);
            H5Dclose(vertsID);
            H5Gclose(domainGID);
            H5Fclose(fileID);
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Expected 3-component Vertices in Domain group in file " << filename << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since Vertices array is not 3-component.");
        }
        
        H5Gclose(domainGID);
        unstructured = true;
    }


    // Look for groups starting with "Time:".  They're our timesteps.
    hsize_t nObjs;
    H5Gget_num_objs(fileID, &nObjs);
    nTime = 0;
    times.clear();
    for(size_t i=0;i<nObjs;i++)
    {
        char name[256];
        H5Gget_objname_by_idx(fileID, i, name, 256);
        if (strncmp(name, "Coordinates", 11) == 0)
            continue;
        if (strncmp(name, "Domain", 6) == 0)
            continue;
        if (strncmp(name, "Time: ", 6) == 0)
        {
            nTime++;
            double time;
            sscanf(name, "Time: %lf h", &time);
            times.push_back(std::make_pair(time,std::string(name)));
        }
        else
        {
            // Unknown name.
            debug2 << "avtPFLOTRANFileFormat::LoadFile: Unknown object in file: " << name << endl;
        }
    }
    std::sort(times.begin(), times.end());

    if (!unstructured)
        DoDomainDecomposition();

    opened = true;
}

// ****************************************************************************
//  Method: avtPFLOTRANFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Wed Jun 15 14:55:51 EDT 2011
//    Move file-closing code from here to destructor.  This function is called
//    when changing time steps; we don't want to close the file then.
//
// ****************************************************************************

void
avtPFLOTRANFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method:  avtPFLOTRANFileFormat::DoDomainDecomposition
//
//  Purpose:
//    Split the single big dataset into parallel chunks.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  1, 2008
//
// ****************************************************************************
void
avtPFLOTRANFileFormat::DoDomainDecomposition()
{
#ifdef PARALLEL
    int zoneDims[3] = {globalDims[0]-1,globalDims[1]-1,globalDims[2]-1};
    avtDatabase::ComputeRectilinearDecomposition(3,
                                                 PAR_Size(),
                                                 zoneDims[0],
                                                 zoneDims[1],
                                                 zoneDims[2],
                                                 &domainCount[0],
                                                 &domainCount[1],
                                                 &domainCount[2]);

    avtDatabase::ComputeDomainLogicalCoords(3, domainCount, PAR_Rank(),
                                            domainIndex);


    for (int axis = 0; axis < 3; axis++)
    {
        avtDatabase::ComputeDomainBounds(zoneDims[axis],
                                         domainCount[axis],
                                         domainIndex[axis],
                                         &domainGlobalStart[axis],
                                         &domainGlobalCount[axis]);
        domainGlobalCount[axis]++; // the returned bounds are zonal; we want nodal

        // start with the assumption that all cells are real
        localRealStart[axis] = 0;
        localRealCount[axis] = domainGlobalCount[axis];

        // now account for ghost cells by expanding the requested cells
        // and updating the local real cell indices
        if (domainIndex[axis] > 0)
        {
            localRealStart[axis]++;
            domainGlobalStart[axis]--;
            domainGlobalCount[axis]++;
        }
        if (domainIndex[axis] < domainCount[axis]-1)
        {
            domainGlobalCount[axis]++;
        }
        debug4 << "PFLOTRAN domain local indices: "
               << "start[" << axis << "] = " << domainGlobalStart[axis]
               << ",  count[" << axis << "] = " << domainGlobalCount[axis]
               << endl;
    }
#else
    domainGlobalStart[0] = 0;
    domainGlobalStart[1] = 0;
    domainGlobalStart[2] = 0;
    domainGlobalCount[0] = globalDims[0];
    domainGlobalCount[1] = globalDims[1];
    domainGlobalCount[2] = globalDims[2];
    localRealStart[0] = 0;
    localRealStart[1] = 0;
    localRealStart[2] = 0;
    localRealCount[0] = globalDims[0];
    localRealCount[1] = globalDims[1];
    localRealCount[2] = globalDims[2];
#endif
}


// ****************************************************************************
//  Method:  avtPFLOTRANFileFormat::AddGhostCellInfo
//
//  Purpose:
//    Add the ghost cell information and local-to-global translations.
//
//  Note:  some of this was borrowed from avtSiloFileFormat::GetQuadGhostZones
//
//  Arguments:
//    ds         the dataset to which we need add the informatio
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  1, 2008
//
// ****************************************************************************
void
avtPFLOTRANFileFormat::AddGhostCellInfo(vtkDataSet *ds)
{
#ifndef PARALLEL
    return;
#endif

    int nx = domainGlobalCount[0];
    int ny = domainGlobalCount[1];
    int nz = domainGlobalCount[2];
    int nnodes = nx*ny*nz;

    bool *ghostPoints = new bool[nnodes];

    //
    // Initialize as all ghost levels
    //
    for (int ii = 0; ii < nnodes; ii++)
        ghostPoints[ii] = true; 

    //
    // Set real values
    //
    for (int k = 0; k < localRealCount[2]; k++)
        for (int j = 0; j < localRealCount[1]; j++)
            for (int i = 0; i < localRealCount[0]; i++)
            {
                int ii = localRealStart[0] + i;
                int jj = localRealStart[1] + j;
                int kk = localRealStart[2] + k;
                int index = kk*ny*nx + jj*nx + ii;
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

    // Add the min/max local logical extents of this domain.  It's
    // an alternate way we label ghost zones for structured grids.
    vtkIntArray *realDims = vtkIntArray::New();
    realDims->SetName("avtRealDims");
    realDims->SetNumberOfValues(6);

    realDims->SetValue(0, localRealStart[0]);
    realDims->SetValue(1, localRealStart[0]+localRealCount[0]-1);

    realDims->SetValue(2, localRealStart[1]);
    realDims->SetValue(3, localRealStart[1]+localRealCount[1]-1);

    realDims->SetValue(4, localRealStart[2]);
    realDims->SetValue(5, localRealStart[2]+localRealCount[2]-1);

    ds->GetFieldData()->AddArray(realDims);
    ds->GetFieldData()->CopyFieldOn("avtRealDims");
    realDims->Delete();

    // And add base_index, the global index of the first real
    // zone in the domain.  This is how we convert from local
    // to global indices.
    vtkIntArray *baseIndex = vtkIntArray::New();
    baseIndex->SetName("base_index");
    baseIndex->SetNumberOfValues(3);
    baseIndex->SetValue(0, domainGlobalStart[0]+localRealStart[0]);
    baseIndex->SetValue(1, domainGlobalStart[1]+localRealStart[1]);
    baseIndex->SetValue(2, domainGlobalStart[2]+localRealStart[2]);
    ds->GetFieldData()->AddArray(baseIndex);
    ds->GetFieldData()->CopyFieldOn("base_index");    

    ds->GetInformation()->Set(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0); 
}


// ****************************************************************************
//  Method: avtPFLOTRANFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  3 12:26:26 EDT 2009
//    All variables are supposed to be cell-centered.
//    Add support for old-style PFLOTRAN files where the coordinate
//    arrays represented cell centers.
//
//    Daniel Schep, Thu Aug 26 15:30:18 EDT 2010
//    Added support for returning vector and material data.
//
//    Jeremy Meredith, Wed Mar 30 12:58:02 EDT 2011
//    Close any HDF5 group/dataset we open.
//
//    Jeremy Meredith, Wed Jun 15 15:56:44 EDT 2011
//    More aggressive about closing ids, including spaces.
//
//    Jeremy Meredith, Wed Dec 19 12:54:03 EST 2012
//    Support unstructured grids (which cannot do their own domain decomp).
//
//    Jeremy Meredith, Wed Feb 13 15:34:24 EST 2013
//    Allow "Material ID" as an alternative to "Material_ID".
//
// ****************************************************************************

void
avtPFLOTRANFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData * md,
                                                 int timeState)
{
    LoadFile();

    if (!unstructured)
        md->SetFormatCanDoDomainDecomposition(true);

    // Mesh
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = 1;  // must be 1 for automatic decomposition
    mesh->blockOrigin = 1;
    mesh->cellOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->blockTitle = "blocks";
    mesh->blockPieceName = "block";
    mesh->hasSpatialExtents = false;
    //mesh->xUnits = "m";
    //mesh->yUnits = "m";
    //mesh->zUnits = "m";
    md->Add(mesh);

    // Look in the timestep for the list of variables.
    hid_t timeGID = H5Gopen(fileID, times[timeState].second.c_str());
    hsize_t nObjs;
    H5Gget_num_objs(timeGID, &nObjs);
    for(size_t i=0;i<nObjs;i++)
    {
        char name[512];
        H5Gget_objname_by_idx(timeGID, i, name, 512);

        // Check if variable is a Vector component
        char* vecMatch;
        for (int comp=0;comp<3;comp++)
            if ((vecMatch = strstr(name,vecNames[comp].c_str()))) break;
        
        // if so, add component to vector list
        if (vecMatch)
        {
            char vectorName[512];
            strncpy(vectorName, name, strlen(name)-strlen(vecMatch));
            strcpy(vectorName+strlen(name)-strlen(vecMatch), vecMatch+2);

            if (vectors.count(string(vectorName)) == 0)
                vectors[string(vectorName)] = vector<string>();
            vectors[string(vectorName)].push_back(string(name));

            continue; // Don't add vectors components as scalars.
        }

        hid_t ds = H5Dopen(timeGID, name);
        hid_t dsSpace = H5Dget_space(ds);
        int ndims = H5Sget_simple_extent_ndims(dsSpace);
        if ((unstructured && ndims != 1) ||
            (!unstructured && ndims != 3))
        {
            H5Sclose(dsSpace);
            H5Dclose(ds);
            continue; // skip it
        }


        // set metadata for Materials
        if(strstr(name, "Material_ID") ||
           strstr(name, "Material ID"))
        {
            int *matlist;
            hsize_t nvals;
            if (unstructured)
            {
                hsize_t maxvals;
                H5Sget_simple_extent_dims(dsSpace, &nvals, &maxvals);

                matlist = new int[nvals];
                herr_t err = H5Dread(ds, H5T_NATIVE_INT,
                                     H5S_ALL, H5S_ALL, H5P_DEFAULT, matlist);
                (void) err;
            }
            else
            {
                hid_t slabSpace = H5Scopy(dsSpace);
                hsize_t start[] = {static_cast<hsize_t>(domainGlobalStart[0]),
                                   static_cast<hsize_t>(domainGlobalStart[1]),
                                   static_cast<hsize_t>(domainGlobalStart[2])};
                hsize_t count[] = {static_cast<hsize_t>(domainGlobalCount[0]-1),
                                   static_cast<hsize_t>(domainGlobalCount[1]-1),
                                   static_cast<hsize_t>(domainGlobalCount[2]-1)};
                if (oldFileNeedingCoordFixup)
                {
                    for (int dim=0; dim<3; dim++)
                        count[dim]++;
                }
                H5Sselect_hyperslab(slabSpace, H5S_SELECT_SET, start, NULL, count, NULL);

                hsize_t beg[3];
                hsize_t end[3];
                H5Sget_select_bounds(slabSpace, beg, end);
                int nx = end[0]-beg[0]+1;
                int ny = end[1]-beg[1]+1;
                int nz = end[2]-beg[2]+1;
                nvals = nx*ny*nz;

                int dims[3]; (void) dims;
                dims[0] = nx;
                dims[1] = ny;
                dims[2] = nz;

                hid_t memSpace = H5Screate_simple(3,count,NULL);

                matlist = new int[nvals];

                int *in = new int[nvals];
                herr_t err = H5Dread(ds, H5T_NATIVE_INT, memSpace, slabSpace,
                                     H5P_DEFAULT, in);
                (void) err;
                H5Sclose(memSpace);
                H5Sclose(slabSpace);

                // Input is in a different ordering (Fortran) than VTK wants (C).
                for (int i=0;i<nx;i++)
                    for (int j=0;j<ny;j++)
                        for (int k=0;k<nz;k++)
                            matlist[k*nx*ny + j*nx + i] = in[k + j*nz + i*nz*ny];
                delete [] in;
            }
            
            H5Sclose(dsSpace);
            H5Dclose(ds);

            // find the different materials
            map<int,bool> matls;
            for (size_t i=0;i<nvals;i++)
                matls[matlist[i]] = true;

            int nmats = (int)matls.size();
            char **names = new char*[nmats];
            int i = 0;
            for (map<int,bool>::iterator iter = matls.begin(); iter != matls.end();++iter)
            {
                names[i] = new char[8];
                sprintf(names[i], "%d", iter->first);
                ++i;
            }


            avtMaterialMetaData *material = new avtMaterialMetaData();
            material->name = name;
            material->meshName = "mesh";
            material->numMaterials = nmats;
            for(int i=0;i<nmats;++i)
                material->materialNames.push_back(names[i]);
            md->Add(material);

     
            delete [] matlist;
            for(int i=0;i<nmats;++i)
                delete [] names[i];
            delete [] names;

            continue; // Don't add materials as a scalar.
        }

        // We need to check if the coordinate array dims are the same
        // as the variables.  So well save off one of the variable's
        // dimensions and compare it with the mesh coordinate array later
        // to see if we need to recenter the coordinate arrays.
        if (!unstructured && i == 0)
        {
            hsize_t varDimsForOldFileTest[3];
            H5Sget_simple_extent_dims(dsSpace, varDimsForOldFileTest, NULL);
            oldFileNeedingCoordFixup = ((hsize_t)globalDims[0]==varDimsForOldFileTest[0]);
        }        

        H5Dclose(ds);

        avtScalarMetaData *scalar = new avtScalarMetaData();
        scalar->name = name;
        scalar->meshName = "mesh";
        scalar->centering = AVT_ZONECENT;
        md->Add(scalar);
    }
    H5Gclose(timeGID);

    //add metadata for vectors if they have 3 components
    for (map<string, vector<string> >::iterator iter=vectors.begin();
        iter!=vectors.end(); ++iter )
    {
        if (iter->second.size() == 3)
        {
            avtVectorMetaData *vector = new avtVectorMetaData();
            vector->name = iter->first;
            vector->meshName = "mesh";
            vector->centering = AVT_ZONECENT;
            md->Add(vector);
        }
        // ignore < 3-vectors
    }


#ifdef PARALLEL
    if (oldFileNeedingCoordFixup)
        EXCEPTION1(ImproperUseException,
                   "Can't support old PFLOTRAN files, needing coordinate "
                   "array recentering, in parallel.");
#endif

    // Set the time information.
    vector<double> t;
    for(int i=0;i<nTime;i++)
        t.push_back(times[i].first);
    md->SetTimes(t);
    md->SetTimesAreAccurate(true);
    md->SetTemporalExtents(times[0].first, times[nTime-1].first);
}

// ****************************************************************************
//  Method: avtPFLOTRANFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  1 12:43:11 EDT 2008
//    Added support for automatic parallel decomposition and parallel I/O
//    via hyperslab reading.
//
//    Jeremy Meredith, Fri Apr  3 12:26:26 EDT 2009
//    All variables are supposed to be cell-centered.
//    Add support for old-style PFLOTRAN files where the coordinate
//    arrays represented cell centers.
//
//    Jeremy Meredith, Wed Jun 15 15:56:44 EDT 2011
//    More aggressive about closing ids, including spaces.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.
//
// ****************************************************************************

vtkDataSet *
avtPFLOTRANFileFormat::GetMesh(int, int domain, const char *)
{
    // Make sure the file is opened and ready.
    LoadFile();

    if (unstructured)
    {
        vtkPoints *points  = vtkPoints::New();
        points->SetDataTypeToDouble();
        points->SetNumberOfPoints(ucd_nverts);
        H5Dread(vertsID, H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,
                (double*)(points->GetVoidPointer(0)));

        vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
        mesh->SetPoints(points);
        points->Delete();

        int *connectivity = new int[ucd_cellstride * ucd_ncells];
        H5Dread(cellsID, H5T_NATIVE_INT,H5S_ALL,H5S_ALL,H5P_DEFAULT,
                connectivity);

        vtkIdType ids[8];
        for (int i=0; i<ucd_ncells; i++)
        {
            int nids = connectivity[ucd_cellstride * i + 0];
            int celltype = 0;
            switch (nids)
            {
              case 4: celltype = VTK_TETRA;      break;
              case 5: celltype = VTK_PYRAMID;    break;
              case 6: celltype = VTK_WEDGE;      break;
              case 8: celltype = VTK_HEXAHEDRON; break;
              default: EXCEPTION1(InvalidVariableException, "mesh");
            }
            // copy the ids, and switch to 0-origin
            for (int j=0; j<nids; j++)
                ids[j] = connectivity[ucd_cellstride * i + 1 + j] - 1;
            mesh->InsertNextCell(celltype, nids, ids);
        }

        delete[] connectivity;

        return mesh;
    }
    else
    {
        // Read the coordinates arrays.
        vtkDoubleArray *localCoords[3];
        for (int dim=0;dim<3;dim++)
        {
            localCoords[dim] = vtkDoubleArray::New();
            localCoords[dim]->SetNumberOfTuples(domainGlobalCount[dim]);
            double *coords = (double*)localCoords[dim]->GetVoidPointer(0);

            hid_t arraySpace = H5Dget_space(dimID[dim]);
            hid_t slabSpace = H5Scopy(arraySpace);
            hsize_t start[] = {static_cast<hsize_t>(domainGlobalStart[dim])};
            hsize_t count[] = {static_cast<hsize_t>(domainGlobalCount[dim])};
            H5Sselect_hyperslab(slabSpace, H5S_SELECT_SET, start,NULL,count,NULL);

            hid_t memSpace = H5Screate_simple(1,count,NULL);

            herr_t err = H5Dread(dimID[dim], H5T_NATIVE_DOUBLE,
                                 memSpace, slabSpace, H5P_DEFAULT, coords);
            (void) err;
            H5Sclose(memSpace);
            H5Sclose(slabSpace);
            H5Sclose(arraySpace);
        }

        vtkRectilinearGrid *mesh = vtkRectilinearGrid::New();

        if (oldFileNeedingCoordFixup)
        {
            int dims[3] = {domainGlobalCount[0],
                           domainGlobalCount[1],
                           domainGlobalCount[2]};
            for (int dim=0; dim<3; dim++)
            {
                if (dims[dim] < 2)
                    continue;

                dims[dim]++;
                int d = dims[dim];
                vtkDoubleArray *oc = localCoords[dim];
                vtkDoubleArray *nc = vtkDoubleArray::New();

                nc->SetNumberOfTuples(oc->GetNumberOfTuples()+1);
                nc->SetTuple1(0, (oc->GetTuple1(0) -
                                  (oc->GetTuple1(1)-oc->GetTuple1(0))/2.));
                for (int i=1; i<d-1; i++)
                    nc->SetTuple1(i, (oc->GetTuple1(i-1)+oc->GetTuple1(i))/2.);
                nc->SetTuple1(d-1, (oc->GetTuple1(d-2) +
                                    (oc->GetTuple1(d-2)-oc->GetTuple1(d-3))/2.));

                localCoords[dim] = nc;
                oc->Delete();
            }

            mesh->SetDimensions(dims);
        }
        else
        {
            mesh->SetDimensions(domainGlobalCount);
        }

        mesh->SetXCoordinates(localCoords[0]);
        mesh->SetYCoordinates(localCoords[1]);
        mesh->SetZCoordinates(localCoords[2]);
        localCoords[0]->Delete();
        localCoords[1]->Delete();
        localCoords[2]->Delete();

        AddGhostCellInfo(mesh);

        return mesh;
    }
}

// ****************************************************************************
//
//  Method: avtPFLOTRANFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  1 12:43:11 EDT 2008
//    Added support for automatic parallel decomposition and parallel I/O
//    via hyperslab reading.
//
//    Jeremy Meredith, Tue Mar 31 11:09:37 EDT 2009
//    Don't try reading integral data into doubles; instead read to ints,
//    the convert to double using the compiler.  At least in the version
//    of HDF5 I used, 1.6.5 x64, trying to read an integer array to a double
//    looked like it worked, but gave garbage for much of the last portion
//    of data.  The HDF5 docs for 1.6 claim they can't do this operation
//    at all, so while I'm surprised it looked even partly okay, I'm
//    confident reading to ints is the correct solution.
//
//    Jeremy Meredith, Fri Apr  3 12:26:26 EDT 2009
//    All variables are supposed to be cell-centered.
//    Add support for old-style PFLOTRAN files where the coordinate
//    arrays represented cell centers.
//
//    Jeremy Meredith, Wed Mar 30 12:58:02 EDT 2011
//    Close any HDF5 group/dataset we open.
//
//    Jeremy Meredith, Wed Jun 15 15:56:44 EDT 2011
//    More aggressive about closing ids, including spaces.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.
//
// ****************************************************************************

vtkDataArray *
avtPFLOTRANFileFormat::GetVar(int timestate, int, const char *varname)
{
    // Make sure the file is opened and ready.
    LoadFile();

    if (unstructured)
    {
        // it's much simpler; we have no decomposition to worry about

        hid_t ts = H5Gopen(fileID, times[timestate].second.c_str());
        hid_t ds = H5Dopen(ts, varname);
        hid_t dsSpace = H5Dget_space(ds);
        int ndims = H5Sget_simple_extent_ndims(dsSpace);
        if (ndims != 1)
        {
            H5Sclose(dsSpace);
            H5Dclose(ds);
            H5Gclose(ts);
            debug1 << "The variable " << varname << " was not 1-dimensional" << endl;
            EXCEPTION1(InvalidVariableException, varname);
        }
        hsize_t nvals, maxvals;
        H5Sget_simple_extent_dims(dsSpace, &nvals, &maxvals);

        vtkDataArray *arr = NULL;

        hid_t intype = H5Dget_type(ds);
        if (H5Tequal(intype, H5T_NATIVE_FLOAT) ||
            H5Tequal(intype, H5T_NATIVE_DOUBLE) ||
            H5Tequal(intype, H5T_NATIVE_LDOUBLE))
        {        
            vtkDoubleArray *array = vtkDoubleArray::New();
            array->SetNumberOfTuples(nvals);
            double *ptr = (double*)array->GetVoidPointer(0);

            herr_t err = H5Dread(ds, H5T_NATIVE_DOUBLE,
                                 H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr);
            (void) err;
            arr = array;
        }
        else
        {
            vtkIntArray *array = vtkIntArray::New();
            array->SetNumberOfTuples(nvals);
            double *ptr = (double*)array->GetVoidPointer(0);

            herr_t err = H5Dread(ds, H5T_NATIVE_INT,
                                 H5S_ALL, H5S_ALL, H5P_DEFAULT, ptr);
            (void) err;
        }   

        H5Sclose(dsSpace);
        H5Dclose(ds);
        H5Gclose(ts);

        return arr;
    }
    else
    {
        // Read the dataset from the file.
        hid_t ts = H5Gopen(fileID, times[timestate].second.c_str());
        hid_t ds = H5Dopen(ts, varname);
        hid_t dsSpace = H5Dget_space(ds);
        int ndims = H5Sget_simple_extent_ndims(dsSpace);
        if (ndims != 3)
        {
            H5Sclose(dsSpace);
            H5Dclose(ds);
            H5Gclose(ts);
            debug1 << "The variable " << varname << " had only " << ndims << " dimensions" << endl;
            EXCEPTION1(InvalidVariableException, varname);
        }

        hid_t slabSpace = H5Scopy(dsSpace);
        hsize_t start[] = {static_cast<hsize_t>(domainGlobalStart[0]),
                           static_cast<hsize_t>(domainGlobalStart[1]),
                           static_cast<hsize_t>(domainGlobalStart[2])};
        hsize_t count[] = {static_cast<hsize_t>(domainGlobalCount[0]-1),
                           static_cast<hsize_t>(domainGlobalCount[1]-1),
                           static_cast<hsize_t>(domainGlobalCount[2]-1)};
        if (oldFileNeedingCoordFixup)
        {
            for (int dim=0; dim<3; dim++)
                count[dim]++;
        }
        H5Sselect_hyperslab(slabSpace, H5S_SELECT_SET, start, NULL, count, NULL);

        hsize_t beg[3];
        hsize_t end[3];
        H5Sget_select_bounds(slabSpace, beg, end);
        int nx = end[0]-beg[0]+1;
        int ny = end[1]-beg[1]+1;
        int nz = end[2]-beg[2]+1;
        int nvals = nx*ny*nz;

        hid_t memSpace = H5Screate_simple(3,count,NULL);

        // Set up the VTK object.
        vtkDoubleArray *array = vtkDoubleArray::New();
        array->SetNumberOfTuples(nvals);
        double *out = (double*)array->GetVoidPointer(0);

        // Read the data -- read to ints or doubles directly, as
        // some versions of HDF5 seem to have problems converting
        // ints to doubles directly.
        hid_t intype = H5Dget_type(ds);
        if (H5Tequal(intype, H5T_NATIVE_FLOAT) ||
            H5Tequal(intype, H5T_NATIVE_DOUBLE) ||
            H5Tequal(intype, H5T_NATIVE_LDOUBLE))
        {        
            double *in = new double[nvals];
            herr_t err = H5Dread(ds, H5T_NATIVE_DOUBLE, memSpace, slabSpace,
                                 H5P_DEFAULT, in);
            (void) err;
            // Input is in a different ordering (Fortran) than VTK wants (C).
            for (int i=0;i<nx;i++)
                for (int j=0;j<ny;j++)
                    for (int k=0;k<nz;k++)
                        out[k*nx*ny + j*nx + i] = in[k + j*nz + i*nz*ny];

            delete [] in;
        }
        else
        {
            int *in = new int[nvals];
            herr_t err = H5Dread(ds, H5T_NATIVE_INT, memSpace, slabSpace,
                                 H5P_DEFAULT, in);
            (void) err;
            // Input is in a different ordering (Fortran) than VTK wants (C).
            for (int i=0;i<nx;i++)
                for (int j=0;j<ny;j++)
                    for (int k=0;k<nz;k++)
                        out[k*nx*ny + j*nx + i] = in[k + j*nz + i*nz*ny];

            delete [] in;
        }

        H5Sclose(memSpace);
        H5Sclose(slabSpace);
        H5Sclose(dsSpace);
        H5Dclose(ds);
        H5Gclose(ts);

        return array;
    }
}

// ****************************************************************************
//  Method: avtPFLOTRANFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Daniel Schep, Thu Aug 26 15:30:18 EDT 2010
//    Implemented support to return vector data.
//
//    Jeremy Meredith, Wed Mar 30 12:58:02 EDT 2011
//    Close any HDF5 group/dataset we open.
//
//    Jeremy Meredith, Wed Jun 15 15:56:44 EDT 2011
//    More aggressive about closing ids, including spaces.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.
//
//    Kathleen Biagas, Thu Mar 26 10:08:35 PDT 2015
//    Fix segv by moving declaration of 'double *out = NULL' outside of
//    for loop.
// 
// ****************************************************************************

vtkDataArray *
avtPFLOTRANFileFormat::GetVectorVar(int timestate, int domain,
                                     const char *varname)
{
    // Make sure the file is opened and ready.
    LoadFile();

    // Get var component names
    vector<string> varnames = vectors[string(varname)];
    sort(varnames.begin(), varnames.end());

    if (unstructured)
    {
        hid_t ts = H5Gopen(fileID, times[timestate].second.c_str());
        vtkDoubleArray *array = NULL;
        double *out = NULL;
        for(int comp=0; comp<3; comp++)
        {
            hid_t ds = H5Dopen(ts, varnames[comp].c_str());
            hid_t dsSpace = H5Dget_space(ds);
            int ndims = H5Sget_simple_extent_ndims(dsSpace);
            if (ndims != 1)
            {
                H5Dclose(ds);
                H5Gclose(ts);
                debug1 << "The variable " << varname << " had " << ndims << " dimensions" << endl;
                EXCEPTION1(InvalidVariableException, varname);
            }
            hsize_t nvals, maxvals;
            H5Sget_simple_extent_dims(dsSpace, &nvals, &maxvals);

            if (comp == 0)
            {
                array = vtkDoubleArray::New();
                array->SetNumberOfComponents(3);
                array->SetNumberOfTuples(nvals);
                out = (double*)array->GetVoidPointer(0);
            }

            // Read the data -- read to ints or doubles directly, as
            // some versions of HDF5 seem to have problems converting
            // ints to doubles directly.
            hid_t intype = H5Dget_type(ds);
            if (H5Tequal(intype, H5T_NATIVE_FLOAT) ||
                H5Tequal(intype, H5T_NATIVE_DOUBLE) ||
                H5Tequal(intype, H5T_NATIVE_LDOUBLE))
            {        
                double *in = new double[nvals];
                herr_t err = H5Dread(ds, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                                     H5P_DEFAULT, in);
                (void) err;
                for (size_t i=0;i<nvals;i++)
                    out[i*3 + comp] = in[i];

                delete [] in;
            }
            else
            {
                int *in = new int[nvals];
                herr_t err = H5Dread(ds, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                                     H5P_DEFAULT, in);
                (void) err;
                for (size_t i=0;i<nvals;i++)
                    out[i*3 + comp] = in[i];

                delete [] in;
            }
            
            H5Sclose(dsSpace);
            H5Dclose(ds);
        }

        H5Gclose(ts);

        return array;
    }
    else
    {
        hid_t ts = H5Gopen(fileID, times[timestate].second.c_str());
        vtkDoubleArray *array = NULL;
        double *out = NULL;
        for(int comp=0; comp<3; comp++)
        {
            hid_t ds = H5Dopen(ts, varnames[comp].c_str());
            hid_t dsSpace = H5Dget_space(ds);
            int ndims = H5Sget_simple_extent_ndims(dsSpace);
            if (ndims != 3)
            {
                H5Dclose(ds);
                H5Gclose(ts);
                debug1 << "The variable " << varname << " had only " << ndims << " dimensions" << endl;
                EXCEPTION1(InvalidVariableException, varname);
            }

            hid_t slabSpace = H5Scopy(dsSpace);
            hsize_t start[] = {static_cast<hsize_t>(domainGlobalStart[0]),
                               static_cast<hsize_t>(domainGlobalStart[1]),
                               static_cast<hsize_t>(domainGlobalStart[2])};
            hsize_t count[] = {static_cast<hsize_t>(domainGlobalCount[0]-1),
                               static_cast<hsize_t>(domainGlobalCount[1]-1),
                               static_cast<hsize_t>(domainGlobalCount[2]-1)};
            if (oldFileNeedingCoordFixup)
            {
                for (int dim=0; dim<3; dim++)
                    count[dim]++;
            }
            H5Sselect_hyperslab(slabSpace, H5S_SELECT_SET, start, NULL, count, NULL);

            hsize_t beg[3];
            hsize_t end[3];
            H5Sget_select_bounds(slabSpace, beg, end);
            int nx = end[0]-beg[0]+1;
            int ny = end[1]-beg[1]+1;
            int nz = end[2]-beg[2]+1;
            int nvals = nx*ny*nz;

            hid_t memSpace = H5Screate_simple(3,count,NULL);

            // Set up the VTK object.
            if (comp == 0)
            {
                array = vtkDoubleArray::New();
                array->SetNumberOfComponents(3);
                array->SetNumberOfTuples(nvals);
                out = (double*)array->GetVoidPointer(0);
            }

            // Read the data -- read to ints or doubles directly, as
            // some versions of HDF5 seem to have problems converting
            // ints to doubles directly.
            hid_t intype = H5Dget_type(ds);
            if (H5Tequal(intype, H5T_NATIVE_FLOAT) ||
                H5Tequal(intype, H5T_NATIVE_DOUBLE) ||
                H5Tequal(intype, H5T_NATIVE_LDOUBLE))
            {        
                double *in = new double[nvals];
                herr_t err = H5Dread(ds, H5T_NATIVE_DOUBLE, memSpace, slabSpace,
                                     H5P_DEFAULT, in);
                (void) err;
                // Input is in a different ordering (Fortran) than VTK wants (C).
                for (int i=0;i<nx;i++)
                    for (int j=0;j<ny;j++)
                        for (int k=0;k<nz;k++)
                            out[k*nx*ny*3 + j*nx*3 + i*3 + comp] = in[k + j*nz + i*nz*ny];

                delete [] in;
            }
            else
            {
                int *in = new int[nvals];
                herr_t err = H5Dread(ds, H5T_NATIVE_INT, memSpace, slabSpace,
                                     H5P_DEFAULT, in);
                (void) err;
                // Input is in a different ordering (Fortran) than VTK wants (C).
                for (int i=0;i<nx;i++)
                    for (int j=0;j<ny;j++)
                        for (int k=0;k<nz;k++)
                            out[k*nx*ny*3 + j*nx*3 + i*3 + comp] = in[k + j*nz + i*nz*ny];

                delete [] in;
            }

            H5Sclose(memSpace);
            H5Sclose(slabSpace);
            H5Sclose(dsSpace);
            H5Dclose(ds);
        }

        H5Gclose(ts);

        return array;
    }
}


// ****************************************************************************
//  Method: avtPFLOTRANFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Get Auxiliary Data associated with this file.
//
//  Arguments:
//      var        The variable of interest.
//      timestep   The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//      type       The type of auxiliary data requested.
//      args       Optional arguments (unused)
//      df         Reference variable to a Destructor for the data returned.
//
//  Note: The only Auxiliary type supported is AUXILIARY_DATA_MATERIAL.
//
//  Programmer: Daniel Schep
//  Creation:   Thu Aug 26 14:14:09 EDT 2010
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 30 13:17:54 EDT 2011
//    Close any HDF5 group/dataset we open.
//
//    Jeremy Meredith, Wed Dec 19 13:12:27 EST 2012
//    Add unstructured grid support.  Also, close spaces, too.
//
// ****************************************************************************

void      *avtPFLOTRANFileFormat::GetAuxiliaryData(const char *var, int timestep, 
                                    int domain, const char *type, void *args, 
                                    DestructorFunction &df)
{
    void *retval = 0;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        // Make sure the file is opened and ready.
        LoadFile();

        // Read the dataset from the file.
        hid_t ts = H5Gopen(fileID, times[timestep].second.c_str());
        hid_t ds = H5Dopen(ts, var);
        hid_t dsSpace = H5Dget_space(ds);
        int ndims = H5Sget_simple_extent_ndims(dsSpace);
        int dims[3];
        if ((unstructured && ndims != 1) ||
            (!unstructured && ndims != 3))
        {
            H5Sclose(dsSpace);
            H5Dclose(ds);
            H5Gclose(ts);
            debug1 << "The variable " << var << " had " << ndims << " dimensions, which was unexpected" << endl;
            EXCEPTION1(InvalidVariableException, var);
        }

        int *matlist;
        hsize_t nvals;
        if (unstructured)
        {
            hsize_t maxvals;
            H5Sget_simple_extent_dims(dsSpace, &nvals, &maxvals);
            dims[0] = nvals;

            matlist = new int[nvals];
            herr_t err = H5Dread(ds, H5T_NATIVE_INT,
                                 H5S_ALL, H5S_ALL, H5P_DEFAULT, matlist);
            (void) err;
        }
        else
        {
            hid_t slabSpace = H5Scopy(dsSpace);
            hsize_t start[] = {static_cast<hsize_t>(domainGlobalStart[0]),
                               static_cast<hsize_t>(domainGlobalStart[1]),
                               static_cast<hsize_t>(domainGlobalStart[2])};
            hsize_t count[] = {static_cast<hsize_t>(domainGlobalCount[0]-1),
                               static_cast<hsize_t>(domainGlobalCount[1]-1),
                               static_cast<hsize_t>(domainGlobalCount[2]-1)};
            if (oldFileNeedingCoordFixup)
            {
                for (int dim=0; dim<3; dim++)
                    count[dim]++;
            }
            H5Sselect_hyperslab(slabSpace, H5S_SELECT_SET, start, NULL, count, NULL);

            hsize_t beg[3];
            hsize_t end[3];
            H5Sget_select_bounds(slabSpace, beg, end);
            int nx = end[0]-beg[0]+1;
            int ny = end[1]-beg[1]+1;
            int nz = end[2]-beg[2]+1;
            nvals = nx*ny*nz;

            dims[0] = nx;
            dims[1] = ny;
            dims[2] = nz;

            hid_t memSpace = H5Screate_simple(3,count,NULL);

            matlist = new int[nvals];

            int *in = new int[nvals];
            herr_t err = H5Dread(ds, H5T_NATIVE_INT, memSpace, slabSpace,
                                 H5P_DEFAULT, in);
            (void) err;
            H5Sclose(memSpace);
            H5Sclose(slabSpace);

            // Input is in a different ordering (Fortran) than VTK wants (C).
            for (int i=0;i<nx;i++)
                for (int j=0;j<ny;j++)
                    for (int k=0;k<nz;k++)
                        matlist[k*nx*ny + j*nx + i] = in[k + j*nz + i*nz*ny];

            delete [] in;
        }

        map<int,bool> matls;
        for (size_t i=0;i<nvals;i++)
            matls[matlist[i]] = true;

        int nmats = (int)matls.size();
        int *matnos = new int[nmats];
        char **names = new char*[nmats];
        int i = 0;
        for(map<int,bool>::iterator iter = matls.begin(); iter != matls.end();++iter)
        {
            matnos[i] = iter->first;
            names[i] = new char[8];
            sprintf(names[i], "%d", iter->first);
            ++i;
        }

        avtMaterial *mat = new avtMaterial(
            nmats,
            matnos,
            names,
            ndims,
            dims,
            0,
            matlist,
            0,0,0,0,0);


        delete [] matlist;
        delete [] matnos;
        for(int i=0;i<nmats;++i)
            delete [] names[i];
        delete [] names;

        retval = (void*)mat;
        df = avtMaterial::Destruct;

        H5Sclose(dsSpace);
        H5Dclose(ds);
        H5Gclose(ts);

    }

    return retval;
}


// ****************************************************************************
// Method:  avtPFLOTRANFileFormat::GetTimes
//
// Purpose:
//   We also set the times vector in the metadata directly above, but
//   that method doesn't work when time-grouping MT files.
//
// Arguments:
//   t          the time array to populate
//
// Programmer:  Jeremy Meredith
// Creation:    August 23, 2011
//
// ****************************************************************************
void
avtPFLOTRANFileFormat::GetTimes(std::vector<double> &t)
{
    size_t nt = times.size();
    for(size_t i=0;i<nt;i++)
        t.push_back(times[i].first);
}
