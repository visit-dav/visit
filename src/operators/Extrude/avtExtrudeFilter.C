/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//  File: avtExtrudeFilter.C
// ************************************************************************* //

#include <avtExtrudeFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtExtents.h>

#include <BadVectorException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidCellTypeException.h>

// ****************************************************************************
//  Method: avtExtrudeFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

avtExtrudeFilter::avtExtrudeFilter()
{
}


// ****************************************************************************
//  Method: avtExtrudeFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
//  Modifications:
//
// ****************************************************************************

avtExtrudeFilter::~avtExtrudeFilter()
{
}


// ****************************************************************************
//  Method:  avtExtrudeFilter::Create
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

avtFilter *
avtExtrudeFilter::Create()
{
    return new avtExtrudeFilter();
}


// ****************************************************************************
//  Method:      avtExtrudeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

void
avtExtrudeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ExtrudeAttributes*)a;

    avtVector axis(atts.GetAxis()), zaxis(0.,0.,1.);
    
    if ((axis.x == 0. && axis.y == 0. && axis.z == 0.)
#if 0
        || zaxis.dot(axis) == 0.
#endif
       )
    {
        EXCEPTION1(BadVectorException, "Extrusion Axis");
    }

    if(atts.GetLength() <= 0.)
    {
        EXCEPTION1(ImproperUseException, "Length must be greater than 0.");
    }

    if(atts.GetSteps() < 1)
    {
        EXCEPTION1(ImproperUseException, "Steps must be at least 1.");
    }
}


// ****************************************************************************
//  Method: avtExtrudeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtExtrudeFilter with the given
//      parameters would result in an equivalent avtExtrudeFilter.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
// ****************************************************************************

bool
avtExtrudeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ExtrudeAttributes*)a);
}


// ****************************************************************************
//  Method: avtExtrudeFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Extrude filter.
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
//  Modifications:
//    Eric Brugger, Thu Jul 24 13:32:08 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtExtrudeFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    vtkDataSet *out_ds = NULL;

    if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        const double *axis = atts.GetAxis();
        if(axis[0] == 0. && axis[1] == 0. && axis[2] == 1.)
            out_ds = ExtrudeToRectilinearGrid(in_ds);
        else
            out_ds = ExtrudeToStructuredGrid(in_ds);
    }
    else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        out_ds = ExtrudeToStructuredGrid(in_ds);
    }
    else
        out_ds = ExtrudeToUnStructuredGrid(vtkPointSet::SafeDownCast(in_ds));

    if(out_ds != NULL && !atts.GetPreserveOriginalCellNumbers())
    {
        out_ds->GetCellData()->RemoveArray("avtOriginalCellNumbers");
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(out_ds,
        in_dr->GetDomain(), in_dr->GetLabel());

    if (out_ds != NULL)
        out_ds->Delete();

    return out_dr;
}

// ****************************************************************************
// Method: avtExtrudeFilter::CopyVariables
//
// Purpose: 
//   Copy the variables from the old dataset to the new dataset, replicating
//   as needed.
//
// Arguments:
//   in_ds           : The input dataset.
//   out_ds          : The output dataset.
//   nSteps          : The number of times we repeat nodes.
//   cellReplication : Optional count of the number of times a cell's data
//                     must be copied. This is used when we break up a cell
//                     into many cells.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:46:38 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtExtrudeFilter::CopyVariables(vtkDataSet *in_ds, vtkDataSet *out_ds, int nSteps,
    const int *cellReplication) const
{
    vtkCellData *incd   = in_ds->GetCellData();
    vtkCellData *outcd  = out_ds->GetCellData();
    vtkPointData *inpd  = in_ds->GetPointData();
    vtkPointData *outpd = out_ds->GetPointData();

    // Copy variables
    vtkIdType n_out_cells = incd->GetNumberOfTuples() * (nSteps-1);
    outcd->CopyAllocate(incd, n_out_cells);
    vtkIdType destCell = 0;
    for (vtkIdType k = 0 ; k < nSteps-1 ; k++)
    {
        vtkIdType nt = incd->GetNumberOfTuples();
        if(cellReplication == NULL)
        {
            for (vtkIdType srcCell = 0 ; srcCell < nt ; srcCell++)
                outcd->CopyData(incd, srcCell, destCell++);
        }
        else
        {
            for (vtkIdType srcCell = 0 ; srcCell < nt ; srcCell++)
            {
                for(int cr = 0; cr < cellReplication[srcCell]; ++cr)
                    outcd->CopyData(incd, srcCell, destCell++);
            }
        }
    }

    vtkIdType n_out_pts = inpd->GetNumberOfTuples() * nSteps;
    outpd->CopyAllocate(inpd, n_out_pts);
    vtkIdType destPoint = 0;
    for (vtkIdType k = 0 ; k < nSteps ; k++)
    {
        vtkIdType nt = inpd->GetNumberOfTuples();
        for (vtkIdType srcPoint = 0 ; srcPoint < nt ; srcPoint++)
            outpd->CopyData(inpd, srcPoint, destPoint++);
    }
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeToRectilinearGrid
//
// Purpose: 
//   Extrudes a rectilinear grid into a new rectilinear grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns:   A new rectilinear grid containing the extruded mesh. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:47:41 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Fri Aug 24, 16:24:21 MST 2012
//   Preserve coordinate type.
//   
// ****************************************************************************

vtkDataSet *
avtExtrudeFilter::ExtrudeToRectilinearGrid(vtkDataSet *in_ds) const
{
    vtkRectilinearGrid *out_ds  = NULL;
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)in_ds;

    int dims[3]={1,1,1};
    rgrid->GetDimensions(dims);
    if(dims[2] > 1)
    {
        EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
    }
    if(dims[1] == 1)
    {
        EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
    }
    else
    {
        debug5 << "Extrude rectilinear grid to rectilinear grid." << endl;

        dims[2] = atts.GetSteps()+1;

        // Create mesh coordinates.
        vtkDataArray *coords[3] = {NULL, NULL, NULL};
        coords[0] = rgrid->GetXCoordinates()->NewInstance();
        coords[0]->DeepCopy(rgrid->GetXCoordinates());

        coords[1] = rgrid->GetYCoordinates()->NewInstance();
        coords[1]->DeepCopy(rgrid->GetYCoordinates());

        coords[2] = coords[0]->NewInstance();
        coords[2]->SetNumberOfTuples(dims[2]);
        for(int i = 0; i < dims[2]; ++i)
        {
            double t = double(i) / double(dims[2]-1);
            coords[2]->SetTuple1(i, t * atts.GetLength());
        }

        // Make a new mesh
        out_ds = vtkRectilinearGrid::New();
        out_ds->SetDimensions(dims);
        out_ds->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        out_ds->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        out_ds->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        // Copy variables
        CopyVariables(in_ds, out_ds, dims[2]);
    }

    return out_ds;
}

// ****************************************************************************
// Method: avtExtrudeFilter::CreateExtrudedPoints
//
// Purpose: 
//   Create a new vtkPoints object that contains extruded versions of the 
//   input points.
//
// Arguments:
//   oldPoints : The old points that we're extruding.
//   nSteps    : The number of times to copy nodes.
//
// Returns:    A new vtkPoints object with new points.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:49:03 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Fri Aug 24, 16:24:21 MST 2012
//   Preserve coordinate type.
//
// ****************************************************************************

vtkPoints *
avtExtrudeFilter::CreateExtrudedPoints(vtkPoints *oldPoints, int nSteps) const
{
    vtkIdType nOldNodes = oldPoints->GetNumberOfPoints();
    vtkPoints *points = vtkPoints::New(oldPoints->GetDataType());
    points->SetNumberOfPoints(nOldNodes * nSteps);

#define extrude_points(type) \
{ \
    type *pts = (type *) points->GetVoidPointer(0); \
    for(int k = 0; k < nSteps; ++k) \
    { \
        type t = type(k) / type(nSteps-1); \
        avtVector offset(atts.GetAxis()); \
        offset.normalize(); \
        offset *= (atts.GetLength() * t); \
        for(int j = 0; j < nOldNodes; ++j) \
        { \
            double pt[3]; \
            oldPoints->GetPoint(j, pt); \
            *pts++ = pt[0] + offset.x; \
            *pts++ = pt[1] + offset.y; \
            *pts++ = pt[2] + offset.z; \
        } \
    } \
}

    if(oldPoints->GetDataType() == VTK_FLOAT)
    {
        extrude_points(float);
    }
    else if(oldPoints->GetDataType() == VTK_DOUBLE)
    {
        extrude_points(double);
    }
 
    return points;
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeToStructuredGrid
//
// Purpose: 
//   Extrude the input rectilinear or structured data into a structured grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns:    A new structured grid.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:50:10 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Fri Aug 24, 16:24:21 MST 2012
//   Preserve coordinate type.
//   
// ****************************************************************************

vtkDataSet *
avtExtrudeFilter::ExtrudeToStructuredGrid(vtkDataSet *in_ds) const
{
    int dims[3] = {1,1,1};
    vtkPoints *points = NULL;

    // Assemble the coordinates differently according to the input mesh type.
    if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *)in_ds;
        sgrid->GetDimensions(dims);
        if(dims[2] > 1)
        {
            EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
        }
        debug5 << "Extrude structured grid to structured grid." << endl;

        dims[2] = atts.GetSteps()+1;

        points = CreateExtrudedPoints(sgrid->GetPoints(), dims[2]);
    }
    else if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)in_ds;
        rgrid->GetDimensions(dims);
        if(dims[2] > 1)
        {
            EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
        }
        if(dims[1] == 1)
        {
            EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
        }
        debug5 << "Extrude rectilinear grid to structured grid." << endl;

        dims[2] = atts.GetSteps()+1;

        points = vtkVisItUtility::NewPoints(rgrid);
        points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);

#define extrude_coords(type) \
{ \
        type *pts = (type *) points->GetVoidPointer(0); \
        for(int k = 0; k < dims[2]; ++k) \
        { \
            type t = type(k) / type(dims[2]-1); \
            avtVector offset(atts.GetAxis()); \
            offset.normalize(); \
            offset *= (atts.GetLength() * t); \
 \
            for(int j = 0; j < dims[1]; ++j) \
            { \
                double y = rgrid->GetYCoordinates()->GetTuple1(j); \
                for(int i = 0; i < dims[0]; ++i) \
                { \
                    double x = rgrid->GetXCoordinates()->GetTuple1(i); \
 \
                    *pts++ = x + offset.x; \
                    *pts++ = y + offset.y; \
                    *pts++ = offset.z; \
                } \
            } \
        } \
}
        if (points->GetDataType() == VTK_FLOAT)
        {
            extrude_coords(float);
        }
        else if (points->GetDataType() == VTK_DOUBLE)
        {
            extrude_coords(double);
        }
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    // Assemble the mesh from the points.
    vtkStructuredGrid *out_ds = vtkStructuredGrid::New(); 
    out_ds->SetPoints(points);
    out_ds->SetDimensions(dims);
    points->Delete();

    // Copy variables
    CopyVariables(in_ds, out_ds, dims[2]);

    return out_ds;
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeToUnStructuredGrid
//
// Purpose: 
//   Extrude unstructured grids and polydata into a new unstructured grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns:    A new unstructured grid.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:50:53 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtExtrudeFilter::ExtrudeToUnStructuredGrid(vtkPointSet *in_ds) const
{
    int nSteps = atts.GetSteps()+1;
    vtkPoints *points = CreateExtrudedPoints(in_ds->GetPoints(), nSteps);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkIdType npts = in_ds->GetNumberOfPoints();
    vtkIdType ncells = in_ds->GetNumberOfCells();
    ugrid->SetPoints(points);
    points->Delete();

    // Create the extruded connectivity
    vtkIdType n_out_cells = ncells * (nSteps-1);
    ugrid->Allocate(8 * n_out_cells);
    int *cellReplication = NULL;
    for(int k = 0; k < nSteps-1; ++k)
    {
        for (vtkIdType cellid = 0 ; cellid < ncells ; cellid++)
        {
            vtkCell *cell = in_ds->GetCell(cellid);
            int c = cell->GetCellType();
            if (c != VTK_QUAD && c != VTK_TRIANGLE && c != VTK_PIXEL &&
                c != VTK_LINE && c != VTK_POLY_LINE && c != VTK_VERTEX)
            {
                ugrid->Delete();
                delete [] cellReplication;
                EXCEPTION1(InvalidCellTypeException, 
                    "anything but points, lines, polyline, quads, and triangles.");
            }
            vtkIdList *list = cell->GetPointIds();
            vtkIdType verts[8];
            vtkIdType offset = k * npts;

            if (c == VTK_VERTEX)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(0) + offset + npts;
                ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else if(c == VTK_LINE)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(1) + offset + npts;
                verts[3] = list->GetId(0) + offset + npts;
                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
            else if (c == VTK_POLY_LINE)
            {
                // If we're seeing a polyline for the first time then we need to
                // keep track of cell replication (a count of how many cells this
                // cell was broken into) so we can copy the cell-centered variables 
                // properly.
                if(k == 0 && cellReplication == NULL)
                {
                    cellReplication = new int[ncells];
                    for(int r = 0; r < ncells; ++r)
                        cellReplication[r] = (r < cellid) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
                {
                    verts[0] = list->GetId(p)   + offset;
                    verts[1] = list->GetId(p+1) + offset;
                    verts[2] = list->GetId(p+1) + offset + npts;
                    verts[3] = list->GetId(p)   + offset + npts;
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);

                    if(k == 0)
                        cellReplication[cellid]++;
                }

                if(k == 0)
                    cellReplication[cellid]--;
            }
            else if (c == VTK_TRIANGLE)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(2) + offset;
                verts[3] = list->GetId(0) + offset + npts;
                verts[4] = list->GetId(1) + offset + npts;
                verts[5] = list->GetId(2) + offset + npts;
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            }
            else if(c == VTK_QUAD)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(2) + offset;
                verts[3] = list->GetId(3) + offset;
                verts[4] = list->GetId(0) + offset + npts;
                verts[5] = list->GetId(1) + offset + npts;
                verts[6] = list->GetId(2) + offset + npts;
                verts[7] = list->GetId(3) + offset + npts;
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else if(c == VTK_PIXEL)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(3) + offset;
                verts[3] = list->GetId(2) + offset;
                verts[4] = list->GetId(0) + offset + npts;
                verts[5] = list->GetId(1) + offset + npts;
                verts[6] = list->GetId(3) + offset + npts;
                verts[7] = list->GetId(2) + offset + npts;
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }

            if(k == 0 && cellReplication != NULL)
                cellReplication[cellid]++;
        }
    }

    // Copy the variables
    CopyVariables(in_ds, ugrid, nSteps, cellReplication);

    delete [] cellReplication;

    return ugrid;
}

// ****************************************************************************
//  Method: avtRevolveFilter::ExtrudeExtents
//
//  Purpose:
//      Determines the extents of a dataset that has been extruded
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 15:11:07 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

void
avtExtrudeFilter::ExtrudeExtents(double *dbounds) const
{
    avtVector offset(atts.GetAxis());
    offset.normalize();
    offset *= atts.GetLength();

#define eMIN(A,B) (((A)<(B)) ? (A) : (B))
#define eMAX(A,B) (((A)>(B)) ? (A) : (B))

    dbounds[0] = eMIN(dbounds[0], dbounds[0] + offset.x);
    dbounds[1] = eMAX(dbounds[1], dbounds[1] + offset.x);
    dbounds[2] = eMIN(dbounds[2], dbounds[2] + offset.y);
    dbounds[3] = eMAX(dbounds[3], dbounds[3] + offset.y);
    dbounds[4] = eMIN(dbounds[4], dbounds[4] + offset.z);
    dbounds[5] = eMAX(dbounds[5], dbounds[5] + offset.z);
}

// ****************************************************************************
// Method: avtExtrudeFilter::UpdateDataObjectInfo
//
// Purpose: 
//   Update the data object information.
//
// Note:       We update the spatial and topo dimension and update the extents.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 10:51:42 PDT 2011
//
// Modifications:
//    Brad Whitlock, Mon Apr  7 15:55:02 PDT 2014
//    Add filter metadata used in export.
//    Work partially supported by DOE Grant SC0007548.
//   
// ****************************************************************************

void
avtExtrudeFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();

    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()+1);
    if (inAtts.GetSpatialDimension() >= 2)
    {
        outAtts.SetSpatialDimension(3);
    }
    else
    {
        outAtts.SetSpatialDimension(inAtts.GetSpatialDimension()+1);
    }
    outValidity.InvalidateZones();
    outValidity.SetPointsWereTransformed(true);
    outValidity.InvalidateSpatialMetaData();

    //
    // This filter invalidates any transform matrix in the pipeline.
    //
    outAtts.SetCanUseTransform(false);

    //
    // Now extrude the extents.
    //
    double b[6];
    if (inAtts.GetOriginalSpatialExtents()->HasExtents())
    {
        inAtts.GetOriginalSpatialExtents()->CopyTo(b);
        ExtrudeExtents(b);
        outAtts.GetOriginalSpatialExtents()->Set(b);
    }

    if (inAtts.GetThisProcsOriginalSpatialExtents()->HasExtents())
    {
        inAtts.GetThisProcsOriginalSpatialExtents()->CopyTo(b);
        ExtrudeExtents(b);
        outAtts.GetThisProcsOriginalSpatialExtents()->Set(b);
    }

    if (inAtts.GetDesiredSpatialExtents()->HasExtents())
    {
        inAtts.GetDesiredSpatialExtents()->CopyTo(b);
        ExtrudeExtents(b);
        outAtts.GetDesiredSpatialExtents()->Set(b);
    }

    if (inAtts.GetActualSpatialExtents()->HasExtents())
    {
        inAtts.GetActualSpatialExtents()->CopyTo(b);
        ExtrudeExtents(b);
        outAtts.GetActualSpatialExtents()->Set(b);
    }

    if (inAtts.GetThisProcsActualSpatialExtents()->HasExtents())
    {
        inAtts.GetThisProcsActualSpatialExtents()->CopyTo(b);
        ExtrudeExtents(b);
        outAtts.GetThisProcsActualSpatialExtents()->Set(b);
    }

    outAtts.AddFilterMetaData("Extrude");
}
