/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <set>
#include <snprintf.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <vtkCSGGrid.h>

#include <VisItDataInterface_V2.h>

#include <avtGhostData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <PolygonToTriangles.C>
#include <PolyhedralSplit.h>

#include <simv2_CSGMesh.h>
#include <simv2_CurvilinearMesh.h>
#include <simv2_PointMesh.h>
#include <simv2_RectilinearMesh.h>
#include <simv2_UnstructuredMesh.h>
#include <simv2_VariableData.h>

// ****************************************************************************
//  Function:  GetQuadGhostZones
//
//  Purpose:  Add ghost zone information to a dataset.
//    
//  Note:  stolen from the Silo file format method of the same name
//
// ****************************************************************************
static void 
GetQuadGhostZones(int nnodes, int ndims,
                  int *dims, int *min_index, int *max_index,
                  vtkDataSet *ds)
{
    //
    //  Determine if we have ghost points
    //
    int first[3];
    int last[3];
    bool ghostPresent = false;
    bool badIndex = false;
    for (int i = 0; i < 3; i++)
    {
        first[i] = (i < ndims ? min_index[i] : 0);
        last[i]  = (i < ndims ? max_index[i] : 0);

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
        bool *ghostPoints = new bool[nnodes];
        //
        // Initialize as all ghost levels
        //
        for (int ii = 0; ii < nnodes; ii++)
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
// Function: SimV2_CreatePoints
//
// Purpose: 
//   Create a vtkPoints object from various VariableData objects.
//
// Arguments:
//   dims             : The number of dimensions
//   coordMode        : The coordinate mode: separate or interleaved.
//   x                : The handle to the x coordinate array.
//   y                : The handle to the y coordinate array.
//   z                : The handle to the z coordinate array.
//   c                : The handle to the c coordinate array.
//   additionalPoints : The number of additional points to create.
//
// Returns:    A vtkPoints object that contains the coordinates.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 11:14:01 PST 2010
//
// Modifications:
//   Brad Whitlock, Mon Nov 22 16:46:15 PST 2010
//   Fix cut & paste error that caused z coordinates to be zero for doubles.
//
// ****************************************************************************

static vtkPoints *
SimV2_CreatePoints(int ndims, int coordMode, 
    visit_handle x, visit_handle y, visit_handle z, visit_handle c, 
    int additionalPoints)
{
    const char *mName  = "SimV2_CreatePoints: ";
    vtkPoints *points = NULL;

    if(coordMode == VISIT_COORD_MODE_SEPARATE)
    {
        debug4 << mName << "VISIT_COORD_MODE_SEPARATE" << endl;

        // Let's get the VariableData properties. The API guarantees that the
        // arrays will have the same properties so we only get the data 
        // pointer as a unique value.
        visit_handle cHandles[3];
        cHandles[0] = x;
        cHandles[1] = y;
        cHandles[2] = z;
        int owner, dataType, nComps, nTuples;
        void *data[3] = {0,0,0};
        for(int i = 0; i < ndims; ++i)
        {
            if(simv2_VariableData_getData(cHandles[i], owner, dataType, 
                nComps, nTuples, data[i]) == VISIT_ERROR)
            {
                return NULL;
            }
        }

        // We have all of the data. Assemble it.
        points = vtkPoints::New();
        if(ndims == 2)
        {
            debug4 << mName << "ndims == 2" << endl;

            if(dataType == VISIT_DATATYPE_FLOAT)
            {
                debug4 << mName << "float data: nTuples=" << nTuples << endl;
                points->SetNumberOfPoints(nTuples + additionalPoints);
                float *dest = (float *)points->GetVoidPointer(0);
                float *x_src = (float*)data[0];
                float *y_src = (float*)data[1];
                for(int i = 0; i < nTuples; ++i)
                {
                    *dest++ = *x_src++;
                    *dest++ = *y_src++;
                    *dest++ = 0.f;
                }
            }
            else if(dataType == VISIT_DATATYPE_DOUBLE)
            {
                debug4 << mName << "double data: nTuples=" << nTuples << endl;
                points->SetDataTypeToDouble();
                points->SetNumberOfPoints(nTuples + additionalPoints);
                double *dest = (double *)points->GetVoidPointer(0);
                double *x_src = (double*)data[0];
                double *y_src = (double*)data[1];
                for(int i = 0; i < nTuples; ++i)
                {
                    *dest++ = *x_src++;
                    *dest++ = *y_src++;
                    *dest++ = 0.;
                }
            }
            else
            {
                points->Delete();
                EXCEPTION1(ImproperUseException,
                "Coordinate arrays must be float or double.\n");
            }
        }
        else
        {
            debug4 << mName << "ndims == 3" << endl;

            if(dataType == VISIT_DATATYPE_FLOAT)
            {
                debug4 << mName << "float data: nTuples=" << nTuples << endl;
                points->SetNumberOfPoints(nTuples + additionalPoints);
                float *dest = (float *)points->GetVoidPointer(0);
                float *x_src = (float*)data[0];
                float *y_src = (float*)data[1];
                float *z_src = (float*)data[2];
                for(int i = 0; i < nTuples; ++i)
                {
                    *dest++ = *x_src++;
                    *dest++ = *y_src++;
                    *dest++ = *z_src++;
                }
            }
            else if(dataType == VISIT_DATATYPE_DOUBLE)
            {
                debug4 << mName << "float data: nTuples=" << nTuples << endl;
                points->SetDataTypeToDouble();
                points->SetNumberOfPoints(nTuples + additionalPoints);
                double *dest = (double *)points->GetVoidPointer(0);
                double *x_src = (double*)data[0];
                double *y_src = (double*)data[1];
                double *z_src = (double*)data[2];
                for(int i = 0; i < nTuples; ++i)
                {
                    *dest++ = *x_src++;
                    *dest++ = *y_src++;
                    *dest++ = *z_src++;
                }
            }
            else
            {
                points->Delete();
                EXCEPTION1(ImproperUseException,
                "Coordinate arrays must be float or double.\n");
            }
        }
    }
    else if(coordMode == VISIT_COORD_MODE_INTERLEAVED)
    {
        debug4 << mName << "VISIT_COORD_MODE_INTERLEAVED" << endl;
        points = vtkPoints::New();

        int owner, dataType, nComps, nTuples;
        void *data = NULL;
        if(simv2_VariableData_getData(c, owner, dataType, nComps, nTuples, data) != VISIT_ERROR)
        {
            if(ndims == 2)
            {
                debug4 << mName << "ndims == 2" << endl;

                // Copy the 2D data to 3D
                if(dataType == VISIT_DATATYPE_FLOAT)
                {
                    debug4 << mName << "float data: nTuples=" << nTuples << endl;

                    points->SetNumberOfPoints(nTuples + additionalPoints);
                    float *dest = (float *)points->GetVoidPointer(0);
                    float *src = (float*)data;
                    for(int i = 0; i < nTuples; ++i)
                    {
                        *dest++ = *src++;
                        *dest++ = *src++;
                        *dest++ = 0.f;
                    }
                }
                else if(dataType == VISIT_DATATYPE_DOUBLE)
                {
                    debug4 << mName << "double data: nTuples=" << nTuples << endl;

                    points->SetDataTypeToDouble();
                    points->SetNumberOfPoints(nTuples + additionalPoints);
                    double *dest = (double *)points->GetVoidPointer(0);
                    double *src = (double*)data;
                    for(int i = 0; i < nTuples; ++i)
                    {
                        *dest++ = *src++;
                        *dest++ = *src++;
                        *dest++ = 0.;
                    }
                }
                else
                {
                    points->Delete();
                    EXCEPTION1(ImproperUseException,
                    "Coordinate arrays must be float or double.\n");
                }
            }
#ifdef ALLOW_WRAPPED_COORDINATES
            else if(additionalPoints > 0)
#else
            else
#endif
            {
                //
                // Copy the 3D coordinates
                //
                debug4 << mName << "additionalPoints=" << additionalPoints << endl;

                if(dataType == VISIT_DATATYPE_FLOAT)
                {
                    debug4 << mName << "float data: nTuples=" << nTuples << endl;
                    points->SetNumberOfPoints(nTuples + additionalPoints);
                    memcpy(points->GetVoidPointer(0), data, 3 * nTuples * sizeof(float));
                }
                else if(dataType == VISIT_DATATYPE_DOUBLE)
                {
                    debug4 << mName << "double data: nTuples=" << nTuples << endl;
                    points->SetDataTypeToDouble();
                    points->SetNumberOfPoints(nTuples + additionalPoints);
                    memcpy(points->GetVoidPointer(0), data, 3 * nTuples * sizeof(double));
                }
                else
                {
                    points->Delete();
                    EXCEPTION1(ImproperUseException,
                    "Coordinate arrays must be float or double.\n");
                }
            }
#ifdef ALLOW_WRAPPED_COORDINATES
// Eh, this is probably too dangerous anyway...
            else
            {
                //
                // Try wrapping the simulation coordinates directly in VTK objects
                //
                debug4 << mName << "ndims == 3" << endl;

                // Use the 3D data as-is. If VisIt owns the data, we steal
                // the pointer from the VariableData so the VTK data array
                // can own the data.
                int canDelete = 0;
                if(owner == VISIT_OWNER_VISIT)
                {
                    debug4 << mName << "stealing data from simv2" << endl;
                    canDelete = 1;
                    simv2_VariableData_nullData(c);
                }

                // NOTE: Since I'm wrapping the raw memory with a VTK data
                // array, does it introduce these problems?
                //   1. Possible memory free mismatch
                //   2. VTK could be referencing simulation memory that isn't
                //      as enduring as we think it is. The old code used to
                //      always copy the data into vtkPoints, which is safer.

                if(dataType == VISIT_DATATYPE_FLOAT)
                {
                    debug4 << mName << "wrapping float data: nTuples=" << nTuples << endl;
                    vtkFloatArray *wrapper = vtkFloatArray::New();
                    wrapper->SetNumberOfComponents(3);
                    wrapper->SetArray((float *)data, nTuples, canDelete);
                    points->SetData(wrapper);
                    wrapper->Delete();
                }
                else if(dataType == VISIT_DATATYPE_DOUBLE)
                {
                    debug4 << mName << "wrapping double data: nTuples=" << nTuples << endl;
                    vtkDoubleArray *wrapper = vtkDoubleArray::New();
                    wrapper->SetNumberOfComponents(3);
                    wrapper->SetArray((double *)data, nTuples, canDelete);
                    points->SetData(wrapper);
                    wrapper->Delete();
                }
                else
                {
                    points->Delete();
                    EXCEPTION1(ImproperUseException,
                    "Coordinate arrays must be float or double.\n");
                }
            }
#endif
        }
    }

    return points;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Curvilinear
//
// Purpose: 
//   Builds a VTK curvilinear mesh from the simulation mesh.
//
// Arguments:
//   cmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 24 16:41:39 PST 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Curvilinear(visit_handle h)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    int ndims = 0, dims[3]={0,0,0}, coordMode = 0;
    int minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0}, baseIndex[3]={0,0,0};
    visit_handle x,y,z,c;   
    if(simv2_CurvilinearMesh_getCoords(h, &ndims, dims, &coordMode,
                                       &x,&y,&z,&c) == VISIT_ERROR ||
       simv2_CurvilinearMesh_getRealIndices(h, minRealIndex, 
                                       maxRealIndex) == VISIT_ERROR ||
       simv2_CurvilinearMesh_getBaseIndex(h, baseIndex) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Create the points.
    //
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c, 0);

    //
    // Create the VTK objects and connect them up.
    //
    vtkStructuredGrid *sgrid   = vtkStructuredGrid::New(); 
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    sgrid->SetDimensions(dims);

    GetQuadGhostZones(dims[0]*dims[1]*dims[2],
                      ndims,
                      dims,
                      minRealIndex,
                      maxRealIndex,
                      sgrid);

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, baseIndex[0]);
    arr->SetValue(1, baseIndex[1]);
    arr->SetValue(2, baseIndex[2]);
    arr->SetName("base_index");
    sgrid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return sgrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Rectilinear
//
// Purpose: 
//   Builds a VTK rectilinear mesh from the simulation mesh.
//
// Arguments:
//  rmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 11:57:18 PST 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Rectilinear(visit_handle h)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    int ndims = 0, dims[3]={0,0,0};
    int minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0}, baseIndex[3]={0,0,0};
    visit_handle x,y,z,c;   
    if(simv2_RectilinearMesh_getCoords(h, &ndims, &x, &y, &z) == VISIT_ERROR ||
       simv2_RectilinearMesh_getRealIndices(h, minRealIndex, maxRealIndex) == VISIT_ERROR ||
       simv2_RectilinearMesh_getBaseIndex(h, baseIndex) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
            "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Obtain the coordinate data from the opaque objects.
    //
    visit_handle cHandles[3];
    cHandles[0] = x;
    cHandles[1] = y;
    cHandles[2] = z;
    int owner[3]={0,0,0}, dataType[3]={0,0,0}, nComps[3]={1,1,1}, nTuples[3] = {0,0,1};
    void *data[3] = {0,0,0};
    for(int i = 0; i < ndims; ++i)
    {
        if(simv2_VariableData_getData(cHandles[i], owner[i], 
            dataType[i], nComps[i], nTuples[i], data[i]) == VISIT_ERROR)
        {
            EXCEPTION1(ImproperUseException,
                "Could not obtain mesh data using the provided handle.\n");
        }
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
    rgrid->SetDimensions(nTuples);

    //
    // Populate the coordinates.
    //
    vtkDataArray *coords[3] = {0,0,0};
    for(int i = 0; i < 3; ++i)
    {
        if(ndims == 2 && i == 2)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(1);
            coords[i]->SetComponent(0, 0, 0);
        }
        else if(dataType[i] == VISIT_DATATYPE_FLOAT)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(nTuples[i]);
            memcpy(coords[i]->GetVoidPointer(0), data[i], nTuples[i] * sizeof(float));
        }
        else if(dataType[i] == VISIT_DATATYPE_DOUBLE)
        {
            coords[i] = vtkDoubleArray::New();
            coords[i]->SetNumberOfTuples(nTuples[i]);
            memcpy(coords[i]->GetVoidPointer(0), data[i], nTuples[i] * sizeof(double));
        }
        else
        {
            for(int j = 0; j < i; ++j)
            {
                if(coords[i] != 0)
                    coords[i]->Delete();
            }
            EXCEPTION1(ImproperUseException,
                       "Coordinate arrays must be float or double.\n");
        }
    }

    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    GetQuadGhostZones(nTuples[0]*nTuples[1]*nTuples[2],
                      ndims,
                      nTuples,
                      minRealIndex,
                      maxRealIndex,
                      rgrid);

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, baseIndex[0]);
    arr->SetValue(1, baseIndex[1]);
    arr->SetValue(2, baseIndex[2]);
    arr->SetName("base_index");
    rgrid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return rgrid;
}

// ****************************************************************************
// Method: SimV2_UnstructuredMesh_Count_Cells
//
// Purpose: 
//   Examine the connectivity and count the number of regular cells and the 
//   number of polyhedral cells.
//
// Arguments: 
//   umesh           : The unstructured mesh
//   normalCellCount : The count of non-polyhedral cells.
//   polyCount       : The number of polyhedral cells
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 18 16:57:18 PST 2010
//
// Modifications:
//   
// ****************************************************************************

static void
SimV2_UnstructuredMesh_Count_Cells(const int *connectivity, int connectivityLen,
    int &normalCellCount, int &polyCount)
{
    int celltype_npts[10];
    celltype_npts[VISIT_CELL_BEAM]  = 2;
    celltype_npts[VISIT_CELL_TRI]   = 3;
    celltype_npts[VISIT_CELL_QUAD]  = 4;
    celltype_npts[VISIT_CELL_TET]   = 4;
    celltype_npts[VISIT_CELL_PYR]   = 5;
    celltype_npts[VISIT_CELL_WEDGE] = 6;
    celltype_npts[VISIT_CELL_HEX]   = 8;

    polyCount = 0;
    normalCellCount = 0;

    const int *cell = connectivity;
    const int *end = connectivity + connectivityLen;
    while(cell < end)
    {
        int celltype = *cell++;
        if(celltype == VISIT_CELL_POLYHEDRON)
        {
            int nfaces = *cell++;
            for(int i = 0; i < nfaces; ++i)
            {
                int npts = *cell++;
                cell += npts;
            }
            polyCount++;
        }
        else if(celltype >= VISIT_CELL_BEAM && celltype <= VISIT_CELL_POLYHEDRON)
        {
            cell += celltype_npts[celltype];
            normalCellCount++;
        }
        else
        {
            EXCEPTION1(ImproperUseException,
                       "An invalid cell type was provided in the unstructured "
                       "mesh connectivity.\n");
        }
    }
}

// ****************************************************************************
// Method: SimV2_Add_PolyhedralCell
//
// Purpose: 
//   Append the current polyhedral cell to the mesh as tets and pyramids while
//   updating the input cellptr to the location of the next polyhedral face.
//
// Arguments:
//   ugrid   : The unstructured grid to which we're adding a cell.
//   cellptr : The pointer to the face connectivity.
//   nnodes  : The number of original nodes.
//   phIndex : The index of the polyhedral zone.
//
// Returns:    Return the number of cells that the polyhedral cell was broken
//             into.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 18 16:59:12 PST 2010
//
// Modifications:
//   
// ****************************************************************************

static int
SimV2_Add_PolyhedralCell(vtkUnstructuredGrid *ugrid, const int **cellptr, 
    int nnodes, int phIndex, PolyhedralSplit *polyhedralSplit)
{
    const int *cell = *cellptr;

    // cell points at the number of faces in the cell at this point.
    int nfaces = *cell++;
    // Iterate over the faces and get a list of unique points
    std::set<int> uniquePointIds;
    for(int i = 0; i < nfaces; ++i)
    {
        int nPointsInFace = *cell++;
        for(int j = 0; j < nPointsInFace; ++j)
            uniquePointIds.insert(*cell++);
    }

    // Come up with a center point and store it.
    double pt[3] = {0.,0.,0.}, center[3] = {0.,0.,0.};
    vtkPoints *points = ugrid->GetPoints();
    polyhedralSplit->AppendPolyhedralNode(uniquePointIds.size());
    for(std::set<int>::const_iterator it = uniquePointIds.begin();
        it != uniquePointIds.end(); ++it)
    {
        polyhedralSplit->AppendPolyhedralNode(*it);
        points->GetPoint(*it, pt);
        center[0] += pt[0];
        center[1] += pt[1];
        center[2] += pt[2];
    }
    double m = 1. / double(uniquePointIds.size());
    center[0] *= m;
    center[1] *= m;
    center[2] *= m; 
    vtkIdType phCenter = nnodes + phIndex;
    points->SetPoint(phCenter, center);

    // Now, iterate over the faces, adding solid cells for them
    int splitCount = 0;
    cell = *cellptr + 1;
    vtkIdType verts[5];
    for(int i = 0; i < nfaces; ++i)
    {
        int nPointsInFace = *cell++;
        if(nPointsInFace == 3)
        {
            // Add a tet
            verts[0] = cell[2];
            verts[1] = cell[1];
            verts[2] = cell[0];
            verts[3] = phCenter;
            ugrid->InsertNextCell(VTK_TETRA, 4, verts);
            splitCount++;
        }
        else if(nPointsInFace == 4)
        {
            // Add a pyramid
            verts[0] = cell[3];
            verts[1] = cell[2];
            verts[2] = cell[1];
            verts[3] = cell[0];
            verts[4] = phCenter;
            ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
            splitCount++;
        }
        else if(nPointsInFace > 4)
        {
            // Find the face center so we can determine a proxy for a normal.
            double fc[3] = {0., 0., 0.};
            for(int j = 0; j < nPointsInFace; ++j)
            {
                points->GetPoint(cell[j], pt);
                fc[0] += pt[0];
                fc[1] += pt[1];
                fc[2] += pt[2];
            }
            fc[0] /= double(nPointsInFace);
            fc[1] /= double(nPointsInFace);
            fc[2] /= double(nPointsInFace);
            double n[3] = {0.,0.,0.};
            n[0] = center[0] - fc[0];
            n[1] = center[1] - fc[1];
            n[2] = center[2] - fc[2];

            // Tesselate the shape into triangles and add tets. We create
            // a tessellator each time so we can add the face's points to
            // it. This should cause the points to be in the same order as
            // they are in the face.
            vtkPoints *localPts = vtkPoints::New();
            localPts->Allocate(nPointsInFace);
            int *local2Global = new int[nPointsInFace];
            VertexManager           uniqueVerts(localPts);
            simv2PolygonToTriangles tess(&uniqueVerts);
            tess.SetNormal(n);
            tess.BeginPolygon();
            tess.BeginContour();
            for(int j = 0; j < nPointsInFace; ++j)
            {
                local2Global[j] = cell[j];
                tess.AddVertex(points->GetPoint(local2Global[j]));
            }
            tess.EndContour();
            tess.EndPolygon();

            for(int t = 0; t < tess.GetNumTriangles(); ++t)
            {
                int a,b,c;
                tess.GetTriangle(t, a, b, c);
                verts[0] = local2Global[a];
                verts[1] = local2Global[b];
                verts[2] = local2Global[c];
                verts[3] = phCenter;
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                splitCount++;
            }

            localPts->Delete();
            delete [] local2Global;
        }
        
        cell += nPointsInFace;
    }

    *cellptr = cell;

    return splitCount;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Unstructured
//
// Purpose: 
//   Builds a VTK unstructured mesh from the simulation mesh.
//
// Arguments:
//   umesh   : The simulation mesh.
//   h       : The handle that references the real mesh data.
//   phSplit : Return a polyhedral split object if we had polyhedra.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 14:01:25 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Sep  1 09:44:34 PDT 2010
//   I fixed an off by 1 that was pointed out.
//
//   Brad Whitlock, Tue Oct 26 16:26:39 PDT 2010
//   I changed the interface to PolyhedralSplit.
//
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Unstructured(int domain, visit_handle h, PolyhedralSplit **phSplit)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    int ndims = 0, nzones = 0, coordMode = 0, firstRealZone=0, lastRealZone=0;
    visit_handle x,y,z,c,conn;   
    if(simv2_UnstructuredMesh_getCoords(h, &ndims, &coordMode, &x, &y, &z, &c) == VISIT_ERROR ||
       simv2_UnstructuredMesh_getConnectivity(h, &nzones, &conn) == VISIT_ERROR ||
       simv2_UnstructuredMesh_getRealIndices(h, &firstRealZone, &lastRealZone) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Get the connectivity
    //
    int connOwner = 0, connDataType=0, connNComps=0, connectivityLen=0;
    void *connData = 0;
    if(simv2_VariableData_getData(conn, connOwner, 
       connDataType, connNComps, connectivityLen, connData) == VISIT_ERROR)
    {
         EXCEPTION1(ImproperUseException,
             "Could not obtain connectivity data using the provided handle.\n");
    }
    const int *connectivity = (const int *)connData;

    // Count the polyhedral cells so we can allocate more points
    int normalCellCount = 0, polyhedralCellCount = 0;
    SimV2_UnstructuredMesh_Count_Cells(connectivity, connectivityLen, normalCellCount,
        polyhedralCellCount);

    //
    // Create the points.
    //
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c,
                                           polyhedralCellCount);
    int nRealPoints = points->GetNumberOfPoints() - polyhedralCellCount;

    //
    // Create the cells.
    //
    int celltype_npts[10];
    celltype_npts[VISIT_CELL_BEAM]  = 2;
    celltype_npts[VISIT_CELL_TRI]   = 3;
    celltype_npts[VISIT_CELL_QUAD]  = 4;
    celltype_npts[VISIT_CELL_TET]   = 4;
    celltype_npts[VISIT_CELL_PYR]   = 5;
    celltype_npts[VISIT_CELL_WEDGE] = 6;
    celltype_npts[VISIT_CELL_HEX]   = 8;

    int celltype_idtype[10];
    celltype_idtype[VISIT_CELL_BEAM]  = VTK_LINE;
    celltype_idtype[VISIT_CELL_TRI]   = VTK_TRIANGLE;
    celltype_idtype[VISIT_CELL_QUAD]  = VTK_QUAD;
    celltype_idtype[VISIT_CELL_TET]   = VTK_TETRA;
    celltype_idtype[VISIT_CELL_PYR]   = VTK_PYRAMID;
    celltype_idtype[VISIT_CELL_WEDGE] = VTK_WEDGE;
    celltype_idtype[VISIT_CELL_HEX]   = VTK_HEXAHEDRON;

    vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    ugrid->Allocate(normalCellCount + 6 * polyhedralCellCount);
    points->Delete();

    *phSplit = 0;
    PolyhedralSplit *polyhedralSplit = 0;
    if(polyhedralCellCount > 0)
    {
        polyhedralSplit = new PolyhedralSplit;
    }

    // Iterate over the connectivity and add the appropriate cell types
    int numCells = 0;
    int phIndex = 0;
    const int *cell = connectivity;
    const int *end = cell + connectivityLen;
    vtkIdType verts[8];
    bool noConnectivityError = true;
    while(cell < end && numCells < nzones && noConnectivityError)
    {
        int celltype = *cell++;

        if(celltype == VISIT_CELL_POLYHEDRON)
        {
            // Add a polyhedral cell as a collection of smaller normal cells.
            int nsplits = SimV2_Add_PolyhedralCell(ugrid, &cell, nRealPoints, 
                phIndex, polyhedralSplit);
            polyhedralSplit->AppendCellSplits(numCells, nsplits);
            phIndex++;
        }
        else if(celltype >= VISIT_CELL_BEAM && celltype <= VISIT_CELL_HEX)
        {
            // Add a normal cell
            int vtktype = celltype_idtype[celltype];
            int nelempts = celltype_npts[celltype];
            for (int j=0; j<nelempts; j++)
            {
                verts[j] = *cell++;
                noConnectivityError &= (verts[j] >= 0 && verts[j] < nRealPoints);
            }
            ugrid->InsertNextCell(vtktype, nelempts, verts);
        }
        else
        {
            // bad cell type
            noConnectivityError = false;
        }

        ++numCells;
    }

    if (!noConnectivityError)
    {
        delete polyhedralSplit;
        ugrid->Delete();
        char tmp[100];
        SNPRINTF(tmp, 100, "Cell %d's connectivity contained invalid points or "
            "an invalid cell type.", numCells-1);
        EXCEPTION1(ImproperUseException, tmp);
    }
    if (numCells != nzones)
    {
        delete polyhedralSplit;
        ugrid->Delete();
        EXCEPTION1(ImproperUseException,
                   "Number of zones and length of connectivity "
                   "array did not match!");
    }

    //
    // Add the ghost zones to the mesh.
    //
    if (firstRealZone == 0 && lastRealZone == 0 )
    {
        debug5 << "Cannot tell if ghost zones are present because "
               << "min_index & max_index are both zero!" << endl;
    }
    else if (firstRealZone < 0 || firstRealZone >= numCells ||
             lastRealZone  < 0 || lastRealZone  >= numCells ||
             firstRealZone > lastRealZone)
    {
        // bad min or max index
        debug5 << "Invalid min/max index for determining ghost zones:  "
               << "\n\tnumCells: " << numCells
               << "\n\tfirstRealZone: " << firstRealZone
               << "\n\tlastRealZone: " << lastRealZone << endl;
    }
    else if (firstRealZone != 0 || lastRealZone != numCells -1)
    {
        vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
        ghostZones->SetNumberOfTuples(numCells);
        unsigned char *gvals = ghostZones->GetPointer(0);
        unsigned char val = 0;
        avtGhostData::AddGhostZoneType(val, 
            DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        for (int i = 0; i < firstRealZone; i++)
            gvals[i] = val;
        for (int i = firstRealZone; i <= lastRealZone; i++)
            gvals[i] = 0;
        for (int i = lastRealZone+1; i < numCells; i++)
            gvals[i] = val;

        if(polyhedralCellCount > 0)
        {
            vtkDataArray *phgz = polyhedralSplit->ExpandDataArray(ghostZones, true);
            ghostZones->Delete();
            ghostZones = (vtkUnsignedCharArray *)phgz;
        }

        ghostZones->SetName("avtGhostZones");
        ugrid->GetCellData()->AddArray(ghostZones);
        ghostZones->Delete();
        ugrid->SetUpdateGhostLevel(0);
    }

    if(polyhedralCellCount > 0)
    {
        vtkDataArray *originalCells = polyhedralSplit->CreateOriginalCells(domain,
            normalCellCount);
        ugrid->GetCellData()->AddArray(originalCells);
        ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");

        // Return the polyhedral split object.
        *phSplit = polyhedralSplit;
    }

    return ugrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_Point
//
// Purpose: 
//   Builds a VTK point mesh from the simulation mesh.
//
// Arguments:
//   pmesh : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 14:08:14 PST 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_Point(visit_handle h)
{
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    int ndims = 0, coordMode = 0;
    visit_handle x,y,z,c;   
    if(simv2_PointMesh_getCoords(h, &ndims, &coordMode, &x, &y, &z, &c) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Create the points.
    //
    vtkPoints *points = SimV2_CreatePoints(ndims, coordMode, x, y, z, c, 0);

    //
    // Add point cells
    //
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    int npts = points->GetNumberOfPoints();
    ugrid->Allocate(points->GetNumberOfPoints());
    vtkIdType onevertex[1];
    for (int i=0; i<npts; i++)
    {
        onevertex[0] = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

    return ugrid;
}

// ****************************************************************************
// Method: SimV2_GetMesh_CSG
//
// Purpose: 
//    Builds a VTK csg mesh from the simulation mesh.
//
// Arguments:
//   csgm : The simulation mesh.
//
// Returns:    A VTK dataset that represents the simulation mesh.
//
// Note:       Adapted from the Silo reader.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 25 14:32:06 PST 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
SimV2_GetMesh_CSG(visit_handle h)
{
    if(h == VISIT_INVALID_HANDLE)
        return NULL;

    //
    // Obtain the mesh data from the opaque object.
    //
    visit_handle typeflags, leftids, rightids, zonelist, bndtypes, bndcoeffs;
    double min_extents[3]={0.,0.,0.}, max_extents[3]={0.,0.,0.};
    if(simv2_CSGMesh_getRegions(h, &typeflags, &leftids, &rightids) == VISIT_ERROR ||
       simv2_CSGMesh_getZonelist(h, &zonelist) == VISIT_ERROR ||
       simv2_CSGMesh_getBoundaryTypes(h, &bndtypes) == VISIT_ERROR ||
       simv2_CSGMesh_getBoundaryCoeffs(h, &bndcoeffs) == VISIT_ERROR ||
       simv2_CSGMesh_getExtents(h, min_extents, max_extents) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain mesh data using the provided handle.\n");
    }

    //
    // Get the data from the opaque arrays.
    //
    int typeflags_owner=0, typeflags_dataType=0, typeflags_nComps=0, typeflags_nTuples=0;
    void *typeflags_data = 0;
    if(simv2_VariableData_getData(typeflags, typeflags_owner, typeflags_dataType, 
       typeflags_nComps, typeflags_nTuples, typeflags_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain typeflags for CSG mesh.\n");
    }

    int leftids_owner=0, leftids_dataType=0, leftids_nComps=0, leftids_nTuples=0;
    void *leftids_data = 0;
    if(simv2_VariableData_getData(leftids, leftids_owner, leftids_dataType, 
       leftids_nComps, leftids_nTuples, leftids_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain leftids for CSG mesh.\n");
    }

    int rightids_owner=0, rightids_dataType=0, rightids_nComps=0, rightids_nTuples=0;
    void *rightids_data = 0;
    if(simv2_VariableData_getData(rightids, rightids_owner, rightids_dataType, 
       rightids_nComps, rightids_nTuples, rightids_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain rightids for CSG mesh.\n");
    }

    int zonelist_owner=0, zonelist_dataType=0, zonelist_nComps=0, zonelist_nTuples=0;
    void *zonelist_data = 0;
    if(simv2_VariableData_getData(zonelist, zonelist_owner, zonelist_dataType, 
       zonelist_nComps, zonelist_nTuples, zonelist_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain zonelist for CSG mesh.\n");
    }

    int bndtypes_owner=0, bndtypes_dataType=0, bndtypes_nComps=0, bndtypes_nTuples=0;
    void *bndtypes_data = 0;
    if(simv2_VariableData_getData(bndtypes, bndtypes_owner, bndtypes_dataType, 
       bndtypes_nComps, bndtypes_nTuples, bndtypes_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain bndtypes for CSG mesh.\n");
    }

    int bndcoeffs_owner=0, bndcoeffs_dataType=0, bndcoeffs_nComps=0, bndcoeffs_nTuples=0;
    void *bndcoeffs_data = 0;
    if(simv2_VariableData_getData(bndcoeffs, bndcoeffs_owner, bndcoeffs_dataType, 
       bndcoeffs_nComps, bndcoeffs_nTuples, bndcoeffs_data) == VISIT_ERROR)
    {
        EXCEPTION1(ImproperUseException,
                   "Could not obtain bndcoeffs for CSG mesh.\n");
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkCSGGrid *csggrid = vtkCSGGrid::New(); 

    double minX = -10.0, minY = -10.0, minZ = -10.0;
    double maxX =  10.0, maxY =  10.0, maxZ =  10.0;
    // set bounds *before* anything else
    if (!((min_extents[0] == 0.0 && max_extents[0] == 0.0 &&
           min_extents[1] == 0.0 && max_extents[1] == 0.0 &&
           min_extents[2] == 0.0 && max_extents[2] == 0.0) ||
          (min_extents[0] == -DBL_MAX && max_extents[0] == DBL_MAX &&
           min_extents[1] == -DBL_MAX && max_extents[1] == DBL_MAX &&
           min_extents[2] == -DBL_MAX && max_extents[2] == DBL_MAX)))
    {
        minX = min_extents[0];
        maxX = max_extents[0];
        minY = min_extents[1];
        maxY = max_extents[1];
        minZ = min_extents[2];
        maxZ = max_extents[2];
    }
    csggrid->SetBounds(minX, maxX, minY, maxY, minZ, maxZ);

    if (bndcoeffs_dataType == VISIT_DATATYPE_DOUBLE)
        csggrid->AddBoundaries(bndtypes_nTuples, (const int *)bndtypes_data, 
                               bndcoeffs_nTuples, (const double*)bndcoeffs_data);
    else
        csggrid->AddBoundaries(bndtypes_nTuples, (const int *)bndtypes_data, 
                               bndcoeffs_nTuples, (const float*)bndcoeffs_data);

    csggrid->AddRegions(leftids_nTuples,
        (const int *)leftids_data, (const int *)rightids_data,
        (const int *)typeflags_data, 0, 0);

    csggrid->AddZones(zonelist_nTuples, (const int *)zonelist_data);

    return csggrid;
}
