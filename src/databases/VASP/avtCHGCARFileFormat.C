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
//                            avtCHGCARFileFormat.C                           //
// ************************************************************************* //

#include <avtCHGCARFileFormat.h>

#include <float.h>
#include <string>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkIdList.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtMTSDFileFormatInterface.h>

#include <Expression.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <sstream>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

using     std::string;

// ****************************************************************************
//  Method: avtCHGCAR constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
//    Jeremy Meredith, Tue Jul 15 15:41:07 EDT 2008
//    Added support for automatic domain decomposition.
//
// ****************************************************************************

avtCHGCARFileFormat::avtCHGCARFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metadata_read = false;
    values_read = -1;
    values = NULL;

    values_per_line = 0;

    ntimesteps = 0;
    natoms = 0;

    globalZDims[0] = 0;
    globalZDims[1] = 0;
    globalZDims[2] = 0;

    unitCell[0][0] = 1;    unitCell[0][1] = 0;    unitCell[0][2] = 0;
    unitCell[1][0] = 0;    unitCell[1][1] = 1;    unitCell[1][2] = 0;
    unitCell[2][0] = 0;    unitCell[2][1] = 0;    unitCell[2][2] = 1;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 27 11:10:46 EST 2007
//    Don't delete "values" if it's a NULL pointer.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file.
//    values_read is now an int refering to the timestep whose values
//    have been read (or -1 if none).
//
// ****************************************************************************

void
avtCHGCARFileFormat::FreeUpResources(void)
{
    if (values)
        values->Delete();
    values = NULL;
    values_read = -1;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:35:04 EST 2007
//    I just added a bunch of infrastructure to visit to support
//    rectilinear grids in transformed space.  Make use of it
//    here (though this can be disabled by removing the #define at the
//    top of the file for now).
//
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
// ****************************************************************************

void
avtCHGCARFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i, j, k;
    ReadAllMetaData();

    md->SetFormatCanDoDomainDecomposition(true);
    // must set num domains to 1 for automatic decomposition
    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 3,
                                               AVT_RECTILINEAR_MESH);

    for (i=0; i<9; i++)
    {
       mmd->unitCellVectors[i] = unitCell[i/3][i%3];
    }

    if (!is_rectilinear)
    {
        for (i=0; i<4; i++)
        {
            for (j=0; j<4; j++)
            {
                if (i<3 && j<3)
                    mmd->rectilinearGridTransform[i*4+j] = unitCell[j][i];
                else if (i==j)
                    mmd->rectilinearGridTransform[i*4+j] = 1.0;
                else
                    mmd->rectilinearGridTransform[i*4+j] = 0.0;
            }
        }
        mmd->rectilinearGridHasTransform = true;
    }

    double extents[6] = { DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX };
    for (i=0; i<=1; i++)
    {
        for (j=0; j<=1; j++)
        {
            for (k=0; k<=1; k++)
            {
                for (int axis=0; axis<3; axis++)
                {
                    double v = (mmd->unitCellVectors[3*0 + axis] * float(i) +
                                mmd->unitCellVectors[3*1 + axis] * float(j) +
                                mmd->unitCellVectors[3*2 + axis] * float(k));
                    if (extents[2*axis] > v)
                        extents[2*axis] = v;
                    if (extents[2*axis+1] < v)
                        extents[2*axis+1] = v;
                }
            }
        }
    }
    mmd->SetExtents(extents);

    md->Add(mmd);

    AddScalarVarToMetaData(md, "charge", "mesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:35:04 EST 2007
//    I just added a bunch of infrastructure to visit to support
//    rectilinear grids in transformed space.  Make use of it
//    here (when it's not disabled) by returning a unit-size rectilinear
//    grid (i.e. fractional lattice coordinates) instead of cartesian
//    coordinates in a curvilinear grid.
//
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
//    Jeremy Meredith, Tue Jul 15 15:41:07 EDT 2008
//    Added support for automatic domain decomposition.
//
// ****************************************************************************

vtkDataSet *
avtCHGCARFileFormat::GetMesh(int ts, const char *meshname)
{
    if (strcmp(meshname, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

    vtkFloatArray   *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(domainGlobalCount[i]);

        for (int j = 0 ; j < domainGlobalCount[i] ; j++)
        {
            if (is_rectilinear)
            {
                coords[i]->SetComponent(j, 0,
                       unitCell[i][i] * float(domainGlobalStart[i] + j)
                                        / float(globalZDims[i]) );
            }
            else
            {
                coords[i]->SetComponent(j, 0, float(domainGlobalStart[i] + j)
                                        / float(globalZDims[i]) );
            }
        }
    }
    rgrid->SetDimensions(domainGlobalCount);
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    AddGhostCellInfo(rgrid);

    return rgrid;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:37:37 EST 2007
//    Fixed a crash occurring because we returned a vtkDataArray without
//    adding a new reference to it -- it was deleted elsewhere in visit
//    when we didn't know about it.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
// ****************************************************************************

vtkDataArray *
avtCHGCARFileFormat::GetVar(int ts, const char *varname)
{
    ReadValues(ts);

    if (string(varname) == "charge")
    {
        values->Register(NULL);
        return values;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
// ****************************************************************************

vtkDataArray *
avtCHGCARFileFormat::GetVectorVar(int ts, const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::ReadValues
//
//  Purpose:
//    Actually read the values from the file
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file.
//    values_read is now an int refering to the timestep whose values
//    have been read (or -1 if none).  Use saved file positions.
//
//    Jeremy Meredith, Tue Jul 15 15:41:07 EDT 2008
//    Added support for automatic domain decomposition.
//
// ****************************************************************************
void
avtCHGCARFileFormat::ReadValues(int timestep)
{
    if (values_read == timestep)
        return;

    if (values)
    {
        values->Delete();
        values = NULL;
    }

    char tmpbuff[4096];

    OpenFileAtBeginning();
    in.seekg(file_positions[timestep]);

    values_read = timestep;

    int nvalues = domainGlobalCount[0]*domainGlobalCount[1]*domainGlobalCount[2];

    values = vtkDoubleArray::New();
    values->SetNumberOfTuples(nvalues);
    double *ptr = (double *) values->GetVoidPointer(0);

    // Loop over all input values
    for (int k=0; k<globalZDims[2]; k++)
    {
        for (int j=0; j<globalZDims[1]; j++)
        {
            for (int i=0; i<globalZDims[0]; i++)
            {
                // The highest indices are a mirror of the lower ones:
                // If we're at a lowest index, take a slow path for mirroring.
                // Otherwise, there's a faster path below....
                if (i==0 || j==0 || k==0)
                {
                    double val;
                    in >> val;
                    // Loop over all mirroring possibilities.  Note that
                    // fi==fj==fk==0 is the "non-mirrored" case, which is
                    // folded in with the mirrored ones.
                    for (int fi = 0 ; fi<=1; fi++)
                    {
                        if (i!=0 && fi!=0) continue;
                        int ii = (i==0 && fi==1) ? globalZDims[0] : i;
                        for (int fj = 0 ; fj<=1; fj++)
                        {
                            if (j!=0 && fj!=0) continue;
                            int jj = (j==0 && fj==1) ? globalZDims[1] : j;
                            for (int fk = 0 ; fk<=1; fk++)
                            {
                                if (k!=0 && fk!=0) continue;
                                int kk = (k==0 && fk==1) ? globalZDims[2] : k;

                                int li = ii - domainGlobalStart[0];
                                int lj = jj - domainGlobalStart[1];
                                int lk = kk - domainGlobalStart[2];
                                if (li>=0 && li<domainGlobalCount[0] &&
                                    lj>=0 && lj<domainGlobalCount[1] &&
                                    lk>=0 && lk<domainGlobalCount[2])
                                {
                                    int index = (li + domainGlobalCount[0]*
                                                 (lj + domainGlobalCount[1]*
                                                  (lk)));
                                    ptr[index] = val;
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Okay, we're in the fast path; it's an interior value
                    // we're about to read, so it won't be mirrored.

                    // If we're not going to use the value, just dump it to
                    // a char buffer so we don't have to parse it.  This 
                    // helps parallelize the I/O costs (since there's so 
                    // much CPU power devoted to parsing text to numbers).
                    int li = i - domainGlobalStart[0];
                    int lj = j - domainGlobalStart[1];
                    int lk = k - domainGlobalStart[2];
                    if (li>=0 && li<domainGlobalCount[0] &&
                        lj>=0 && lj<domainGlobalCount[1] &&
                        lk>=0 && lk<domainGlobalCount[2])
                    {
                        double val;
                        in >> val;
                        int index = (li + domainGlobalCount[0]*
                                     (lj + domainGlobalCount[1]*
                                      (lk)));
                        ptr[index] = val;
                    }
                    else
                    {
                        in >> tmpbuff;
                    }
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtCHGCARFileFormat::AddGhostCellInfo
//
//  Purpose:
//    Add the ghost cell info to a vtk data set.
//
//  Arguments:
//    ds         the dataset to which we should append the info
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2008
//
// ****************************************************************************

void
avtCHGCARFileFormat::AddGhostCellInfo(vtkDataSet *ds)
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
//  Method:  avtCHGCARFileFormat::DoDomainDecomposition
//
//  Purpose:
//    Find a domain decomposition for this problem based on num processors.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2008
//
// ****************************************************************************
void
avtCHGCARFileFormat::DoDomainDecomposition()
{
#ifdef PARALLEL
    avtDatabase::ComputeRectilinearDecomposition(3,
                                                 PAR_Size(),
                                                 globalZDims[0],
                                                 globalZDims[1],
                                                 globalZDims[2],
                                                 &domainCount[0],
                                                 &domainCount[1],
                                                 &domainCount[2]);

    avtDatabase::ComputeDomainLogicalCoords(3, domainCount, PAR_Rank(),
                                            domainIndex);


    for (int axis = 0; axis < 3; axis++)
    {
        avtDatabase::ComputeDomainBounds(globalZDims[axis],
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
        debug4 << "CHGCAR domain local indices: "
               << "start[" << axis << "] = " << domainGlobalStart[axis]
               << ",  count[" << axis << "] = " << domainGlobalCount[axis]
               << endl;
    }
#else
    domainGlobalStart[0] = 0;
    domainGlobalStart[1] = 0;
    domainGlobalStart[2] = 0;
    domainGlobalCount[0] = globalNDims[0];
    domainGlobalCount[1] = globalNDims[1];
    domainGlobalCount[2] = globalNDims[2];
    localRealStart[0] = 0;
    localRealStart[1] = 0;
    localRealStart[2] = 0;
    localRealCount[0] = globalNDims[0];
    localRealCount[1] = globalNDims[1];
    localRealCount[2] = globalNDims[2];
#endif
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::ReadAllMetaData
//
//  Purpose:
//    Scan the file, quickly looking for metadata.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file.
//    Save file positions for time steps, assuming 10 values per line
//    and an overall file structure matching the examples I've seen so far.
//
//    Hank Childs, Thu Feb 21 16:18:28 PST 2008
//    Initialize variable for the case where parsing fails. (Klocwork)
//
//    Jeremy Meredith, Tue Jul 15 15:41:07 EDT 2008
//    Added support for automatic domain decomposition.
//
//    Jeremy Meredith, Fri Jul 25 10:52:17 EDT 2008
//    Allow for some blank lines before the data starts.  This shouldn't
//    happen with a well-formed CHGCAR file, but the toy data file I
//    created for testing did, and there's no reason not to make the reader
//    more robust to account for it.  (It's an ASCII format, after all.)
//
// ****************************************************************************
void
avtCHGCARFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    metadata_read = true;

    char line[2048];
    in.getline(line, 2048);

    double scale = 0.;
    double lat[3][3];
    in >> scale;
    in >> lat[0][0] >> lat[0][1] >> lat[0][2];
    in >> lat[1][0] >> lat[1][1] >> lat[1][2];
    in >> lat[2][0] >> lat[2][1] >> lat[2][2];

    if (lat[0][1]==0 &&
        lat[0][2]==0 &&
        lat[1][0]==0 &&
        lat[1][2]==0 &&
        lat[2][0]==0 &&
        lat[2][1]==0)
    {
        is_rectilinear = true;
    }
    else
    {
        is_rectilinear = false;
    }

    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            unitCell[i][j] = scale*lat[i][j];

    in.getline(line, 2048); // skip rest of last lattice line
    in.getline(line, 2048); // get atom counts
    string atomcountline(line);

    natoms = 0;
    int tmp = 0;
    std::istringstream count_in(atomcountline);
    while (count_in >> tmp)
    {
        natoms += tmp;
    }

    in.getline(line,2048); // skip next line

    for (int i=0; i<natoms; i++)
    {
        in.getline(line,2048); // skip atom lines
    }
    in.getline(line,2048); // skip blank line

    in >> globalZDims[0] >> globalZDims[1] >> globalZDims[2];
    globalNDims[0] = globalZDims[0]+1;
    globalNDims[1] = globalZDims[1]+1;
    globalNDims[2] = globalZDims[2]+1;

    in.getline(line,2048); // skip rest of dims line

    // count the number of values per line
    istream::pos_type start_of_data = in.tellg();
    string tmpbuff;
    values_per_line = 0;
    int attempts = 0;
    while (values_per_line == 0 && attempts < 10)
    {
        in.getline(line,2048); // skip blank line
        std::istringstream count_values(line);
        while (count_values >> tmpbuff)
            values_per_line++;
        attempts++;
    }

    // Mark the start of the volumetric grid data
    int values_per_vol = globalZDims[0]*globalZDims[1]*globalZDims[2];
    int lines_per_vol = (values_per_vol+values_per_line-1)/values_per_line;
    int lines_per_step = 7 + natoms + 2 + lines_per_vol;

    in.seekg(start_of_data);
    while (in)
    {
        // Note: we're starting partway through the first timestep, so
        // each increment of lines_per_step takes us partway into the middle
        // of the next timestep.  I.e. we should feel safe assuming
        // this next timestep is valid.
        ntimesteps++;
        file_positions.push_back(in.tellg());
        for (int i=0; i<lines_per_step; i++)
            in.getline(line,2048);
    }

    DoDomainDecomposition();
}

// ****************************************************************************
//  Method:  avtCHGCARFileFormat::Identify
//
//  Purpose:
//    Return true if the file given is a CHGCAR VASP file.
//    So far, only check based on the filename.
//
//  Arguments:
//    filename   the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
bool
avtCHGCARFileFormat::Identify(const std::string &filename)
{
    // strip off all leading paths
    int pos = filename.length()-1;
    while (pos>=0 && filename[pos]!='/' && filename[pos]!='\\')
        pos--;

    std::string fn;
    if (pos >= 0)
        fn = filename.substr(pos+1);
    else
        fn = filename;

    // uppercase it
    for (int i=0; i<fn.size(); i++)
    {
        if (fn[i]>='a' && fn[i]<='z')
            fn[i] = fn[i] + ('A'-'a');
    }

    for (int i=0; i<=fn.length()-3; i++)
    {
        if (fn.substr(i,3) == "CHG")
            return true;
    }

    return false;
}

// ****************************************************************************
//  Method:  avtCHGCARFileFormat::CreateInterface
//
//  Purpose:
//    Create a file format interface from this reader.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:11:38 EST 2008
//    Now MTSD.
//
// ****************************************************************************
avtFileFormatInterface *
avtCHGCARFileFormat::CreateInterface(const char *const *list,
                                     int nList, int nBlock)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        ffl[i] = new avtCHGCARFileFormat(list[i]);
    }
    return new avtMTSDFileFormatInterface(ffl, nList);
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  2, 2008
//
// ****************************************************************************
void
avtCHGCARFileFormat::OpenFileAtBeginning()
{
    if (!in.is_open())
    {
        in.open(filename.c_str());
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }
    }
    else
    {
        in.clear();
        in.seekg(0, ios::beg);
    }
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::GetNTimesteps
//
//  Purpose:
//    return the number of timesteps
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  2, 2008
//
//  Modifications:
//
// ****************************************************************************
int
avtCHGCARFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return ntimesteps;
}
