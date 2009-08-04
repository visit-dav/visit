/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <algorithm>
#include <string>
#include <DebugStream.h>
#include <snprintf.h>

#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>

#include <avtDatabaseMetaData.h>
#include <avtDatabase.h>

#include <InvalidDBTypeException.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

using std::string;

static std::string coordNames[3] = { "X [m]", "Y [m]", "Z [m]" };

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
// ****************************************************************************

avtPFLOTRANFileFormat::avtPFLOTRANFileFormat(const char *fname):
    avtMTMDFileFormat(fname)
{
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
// ****************************************************************************

avtPFLOTRANFileFormat::~avtPFLOTRANFileFormat()
{
    free(filename);
    filename = NULL;
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
// ****************************************************************************

void
avtPFLOTRANFileFormat::LoadFile(void)
{
    if (opened == true)
        return;

    // Initialize HDF5.
    H5open();

    // Grab the file and begin.
    fileID = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fileID < 0)
    {
        debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open <" << filename << ">" << endl;
        char error[1024];
        SNPRINTF(error, 1024, "Cannot be a PFLOTRAN file (%s) since it is not even an HDF5 file:",filename);
        EXCEPTION1(InvalidDBTypeException, error);
    }

    // Check the coordinates and pull out the mesh dimensions
    // while we're at it.
    hid_t coordsGID = H5Gopen(fileID, "Coordinates");
    if (coordsGID < 0)
    {
        debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open the Coordinates group in file " << filename << endl;
        EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have a Coordinates group.");
    }
    for (int dim=0;dim<3;dim++)
    {
        dimID[dim] = H5Dopen(coordsGID, coordNames[dim].c_str());
        if (dimID[dim] < 0)
        {
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not open the " << coordNames[dim] << "dataset in file " << filename << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have valid coordinates data.");
        }
        hid_t dimSpaceID = H5Dget_space(dimID[dim]);
        if (dimSpaceID < 0)
        {
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "Could not get the space information for the " << coordNames[dim] << " coordinate in file " << filename << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since it does not have valid coordinates data.");
        }
        int ndims = H5Sget_simple_extent_ndims(dimSpaceID);
        if (ndims != 1)
        {
            debug4 << "avtPFLOTRANFileFormat::LoadFile: " << "The " << coordNames[dim] << " coordinate is not one dimensional" << endl;
            EXCEPTION1(InvalidDBTypeException, "Cannot be a PFLOTRAN file since some coordinate data is not one dimensional.");
        }
        hsize_t dims, maxdims;
        H5Sget_simple_extent_dims(dimSpaceID, &dims, &maxdims);

        globalDims[dim] = dims;
    }

    // Look for groups starting with "Time:".  They're our timesteps.
    hsize_t nObjs;
    H5Gget_num_objs(fileID, &nObjs);
    nTime = 0;
    times.clear();
    for(int i=0;i<nObjs;i++)
    {
        char name[256];
        H5Gget_objname_by_idx(fileID, i, name, 256);
        if (strncmp(name, "Coordinates", 11) == 0)
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
// ****************************************************************************

void
avtPFLOTRANFileFormat::FreeUpResources(void)
{
    if (fileID > 0)
        H5Fclose(fileID);
    opened = false;
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

    ds->SetUpdateGhostLevel(0);
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
// ****************************************************************************

void
avtPFLOTRANFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData * md,
                                                 int timeState)
{
    LoadFile();

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
    //mesh->xUnits = "mm";
    //mesh->yUnits = "mm";
    //mesh->zUnits = "mm";
    md->Add(mesh);

    // Look in the timestep for the list of variables.
    hid_t g = H5Gopen(fileID, times[timeState].second.c_str());
    hsize_t nObjs;
    H5Gget_num_objs(g, &nObjs);
    for(int i=0;i<nObjs;i++)
    {
        char name[512];
        H5Gget_objname_by_idx(g, i, name, 512);

        hid_t ds = H5Dopen(g, name);
        hid_t dsSpace = H5Dget_space(ds);
        int ndims = H5Sget_simple_extent_ndims(dsSpace);
        if (ndims != 3)
            continue; // skip it

        // We need to check if the coordinate array dims are the same
        // as the variables.  So well save off one of the variable's
        // dimensions and compare it with the mesh coordinate array later
        // to see if we need to recenter the coordinate arrays.
        if (i == 0)
        {
            hsize_t varDimsForOldFileTest[3];
            H5Sget_simple_extent_dims(dsSpace, varDimsForOldFileTest, NULL);
            oldFileNeedingCoordFixup = (globalDims[0]==varDimsForOldFileTest[0]);
        }        

        avtScalarMetaData *scalar = new avtScalarMetaData();
        scalar->name = name;
        scalar->meshName = "mesh";
        scalar->centering = AVT_ZONECENT;
        md->Add(scalar);
    }

#ifdef PARALLEL
    if (oldFileNeedingCoordFixup)
        EXCEPTION1(ImproperUseException,
                   "Can't support old PFLOTRAN files, needing coordinate "
                   "array recentering, in parallel.");
#endif

    // Set the time information.
    std::vector<double> t;
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
// ****************************************************************************

vtkDataSet *
avtPFLOTRANFileFormat::GetMesh(int, int domain, const char *)
{
    // Make sure the file is opened and ready.
    LoadFile();

    // Read the coordinates arrays.
    vtkDoubleArray *localCoords[3];
    for (int dim=0;dim<3;dim++)
    {
        localCoords[dim] = vtkDoubleArray::New();
        localCoords[dim]->SetNumberOfTuples(domainGlobalCount[dim]);
        double *coords = (double*)localCoords[dim]->GetVoidPointer(0);

        hid_t arraySpace = H5Dget_space(dimID[dim]);
        hid_t slabSpace = H5Scopy(arraySpace);
        hsize_t start[] = {domainGlobalStart[dim]};
        hsize_t count[] = {domainGlobalCount[dim]};
        H5Sselect_hyperslab(slabSpace, H5S_SELECT_SET, start,NULL,count,NULL);

        hid_t memSpace = H5Screate_simple(1,count,NULL);

        herr_t err = H5Dread(dimID[dim], H5T_NATIVE_DOUBLE,
                             memSpace, slabSpace, H5P_DEFAULT, coords);
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
// ****************************************************************************

vtkDataArray *
avtPFLOTRANFileFormat::GetVar(int timestate, int, const char *varname)
{
    // Make sure the file is opened and ready.
    LoadFile();

    // Read the dataset from the file.
    hid_t ts = H5Gopen(fileID, times[timestate].second.c_str());
    hid_t ds = H5Dopen(ts, varname);
    hid_t dsSpace = H5Dget_space(ds);
    int ndims = H5Sget_simple_extent_ndims(dsSpace);
    if (ndims != 3)
    {
        debug1 << "The variable " << varname << " had only " << ndims << " dimensions" << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }

    hid_t slabSpace = H5Scopy(dsSpace);
    hsize_t start[] = {domainGlobalStart[0],domainGlobalStart[1],domainGlobalStart[2]};
    hsize_t count[] = {domainGlobalCount[0]-1,domainGlobalCount[1]-1,domainGlobalCount[2]-1};
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
        // Input is in a different ordering (Fortran) than VTK wants (C).
        for (int i=0;i<nx;i++)
            for (int j=0;j<ny;j++)
                for (int k=0;k<nz;k++)
                    out[k*nx*ny + j*nx + i] = in[k + j*nz + i*nz*ny];

        delete [] in;
    }

    return array;
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
// ****************************************************************************

vtkDataArray *
avtPFLOTRANFileFormat::GetVectorVar(int timestate, int domain,
                                     const char *varname)
{
    // We don't have any vectors, so we should never get here.
    EXCEPTION1(InvalidVariableException, varname);
}
