// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    
    if (axis.x == 0. && axis.y == 0. && axis.z == 0.)
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
//  Method: avtExtrudeFilter::ModifyContract
//
//  Purpose:
//      This method makes any necessay modification to the VisIt contract
//      to request that the extrude variable is also loaded if it 
//      is not already part of the contract.
//
// ****************************************************************************

avtContract_p
avtExtrudeFilter::ModifyContract(avtContract_p in_contract)
{
    // Create the output contract
    avtContract_p out_contract = new avtContract(in_contract);

    // Add the index variable to the contract if necessay
    if( atts.GetByVariable() && atts.GetVariable() != "default")
        out_contract->GetDataRequest()->AddSecondaryVariable( atts.GetVariable().c_str() );
    else
    {
        // Did not specify an extrude variable so we'll be using the
        // data request's variable. But that could be the name of the
        // mesh in some pipelines. To be safe, request original cells
        // too so they can be used as the index variable when a mesh
        // name is used.
        out_contract->GetDataRequest()->TurnNodeNumbersOn();
    }

    mainVariable = std::string( out_contract->GetDataRequest()->GetVariable() );

    return out_contract;
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
//    Eric Brugger, Tue Aug 25 10:22:23 PDT 2015
//    Modified the routine to return NULL if the output data set was NULL.
//
// ****************************************************************************

avtDataRepresentation *
avtExtrudeFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    // Get the VTK data set.
    vtkDataSet  *in_ds = in_dr->GetDataVTK();
    vtkDataSet *out_ds = NULL;

    nodeData = false;
    cellData = false;

    varArray = nullptr;
    
    // Extrude using a scalar value.
    if( atts.GetByVariable() )
    {
        // Default scalar value
        if( atts.GetVariable() == "default" ) {
          varArray = in_ds->GetPointData()->GetScalars();
          
          if( varArray ) {
            nodeData = true;
          }
          else {
            varArray = in_ds->GetCellData()->GetScalars();

            if( varArray ) {
              cellData = true;
            }
          }
        }
        // Secondary scalar value
        else {
          varArray = in_ds->GetPointData()->GetArray( atts.GetVariable().c_str() );
          
          if( varArray ) {
            nodeData = true;
          }
          else {
            varArray = in_ds->GetCellData()->GetArray( atts.GetVariable().c_str() );

            if( varArray ) {
              cellData = true;
            }
          }
        }
        
        if( varArray == nullptr )
        {
          EXCEPTION1(ImproperUseException,
                     "Unable to locate scalar data for extruding.");
        }
        
        minScalar = 0.0;
        maxScalar = 0.0;
    
        if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
          if( cellData )
            out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkRectilinearGrid::SafeDownCast(in_ds));
          else // if( nodeData )
            out_ds = ExtrudeToStructuredGrid(in_ds);
        }
        else if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
        {
          if( cellData )
            out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
          else // if( nodeData )
            out_ds = ExtrudeToStructuredGrid(in_ds);
        }
        else if(in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
        {
          if( cellData )
            out_ds = ExtrudeCellVariableToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
          else // if( nodeData )
            out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
        }
        else
        {
          EXCEPTION1(ImproperUseException,
                     "Unsupported vtk grid for extruding.");
        }
    }

    // Fix length extrusion.
    else
    {
        minScalar = 1.0;
        maxScalar = 1.0;
    
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
        else if(in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
        {
          out_ds = ExtrudeToUnstructuredGrid(vtkPointSet::SafeDownCast(in_ds));
        }
        else
        {
          EXCEPTION1(ImproperUseException,
                     "Unsupported vtk grid for extruding.");
        }
    }
    
    if(out_ds != NULL && !atts.GetPreserveOriginalCellNumbers())
    {
        out_ds->GetCellData()->RemoveArray("avtOriginalCellNumbers");
    }

    avtDataRepresentation *out_dr = NULL;

    if (out_ds != NULL)
    {
        out_dr = new avtDataRepresentation(out_ds, in_dr->GetDomain(),
                                           in_dr->GetLabel());
        out_ds->Delete();
    }

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
//   nLevels         : The number of times the nodes are repeated (at least 2x)
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
avtExtrudeFilter::CopyVariables(vtkDataSet *in_ds, vtkDataSet *out_ds, int nLevels,
                                const int *cellReplication) const
{
    // Copy cell variables
    vtkCellData *incd   = in_ds->GetCellData();
    vtkCellData *outcd  = out_ds->GetCellData();

    vtkIdType n_out_cells = incd->GetNumberOfTuples() * nLevels;
    outcd->CopyAllocate(incd, n_out_cells);

    vtkIdType destCell = 0;
    
    for (vtkIdType l = 0 ; l < nLevels ; l++)
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

    // Copy node variables
    vtkPointData *inpd  = in_ds->GetPointData();
    vtkPointData *outpd = out_ds->GetPointData();

    // When extruding by a cell variable new points are created for each cell.
    if( atts.GetByVariable() && cellData )
    {
      if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
      {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *)in_ds;

        int dims[3] = {1,1,1};
        rgrid->GetDimensions(dims);

        vtkIdType nCells = (dims[0]-1) * (dims[1]-1);

        vtkIdType n_out_pts = nCells * 8 * nLevels;
        outpd->CopyAllocate(inpd, n_out_pts);
        
        vtkIdType destPoint = 0;

        for(int l=0; l<nLevels; ++l)
        {
          for(int j=0; j<dims[1]-1; ++j)
          {
            for(int i=0; i<dims[0]-1; ++i)
            {
              outpd->CopyData(inpd,  j   *dims[0]+i,   destPoint++ );
              outpd->CopyData(inpd,  j   *dims[0]+i+1, destPoint++ );
              outpd->CopyData(inpd, (j+1)*dims[0]+i+1, destPoint++ );
              outpd->CopyData(inpd, (j+1)*dims[0]+i,   destPoint++ );
                
              outpd->CopyData(inpd,  j   *dims[0]+i,   destPoint++ );
              outpd->CopyData(inpd,  j   *dims[0]+i+1, destPoint++ );
              outpd->CopyData(inpd, (j+1)*dims[0]+i+1, destPoint++ );
              outpd->CopyData(inpd, (j+1)*dims[0]+i,   destPoint++ );
            }
          }
        }
      }
      else
      {
        vtkIdType nCells = in_ds->GetNumberOfCells();

        vtkIdType n_out_pts = nCells * 8 * nLevels;
        outpd->CopyAllocate(inpd, n_out_pts);
        
        vtkIdType destPoint = 0;

        for (vtkIdType l = 0 ; l < nLevels ; l++)
        {
          for (vtkIdType cellid=0; cellid<nCells; ++cellid)
          {
            vtkCell *cell = in_ds->GetCell(cellid);
            int c = cell->GetCellType();
            
            if (c != VTK_QUAD && c != VTK_TRIANGLE && c != VTK_PIXEL &&
                c != VTK_LINE && c != VTK_POLY_LINE && c != VTK_VERTEX)
            {
              EXCEPTION1(InvalidCellTypeException,
                         "anything but points, lines, polyline, quads, and triangles.");
            }
            
            vtkIdList *list = cell->GetPointIds();
            
            if (c == VTK_VERTEX)
            {
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
            }
            else if(c == VTK_LINE)
            {
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              
            }
            else if (c == VTK_POLY_LINE)
            {
              for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
              {
                outpd->CopyData(inpd, list->GetId(p),   destPoint++ );
                outpd->CopyData(inpd, list->GetId(p+1), destPoint++ );
                outpd->CopyData(inpd, list->GetId(p+1), destPoint++ );
                outpd->CopyData(inpd, list->GetId(p),   destPoint++ );
              }
            }
            else if (c == VTK_TRIANGLE)
            {
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(2), destPoint++ );
              
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(2), destPoint++ );
            }
            else if(c == VTK_QUAD)
            {
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(2), destPoint++ );
              outpd->CopyData(inpd, list->GetId(3), destPoint++ );
                  
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(2), destPoint++ );
              outpd->CopyData(inpd, list->GetId(3), destPoint++ );
            }
            else if(c == VTK_PIXEL)
            {
              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(3), destPoint++ );
              outpd->CopyData(inpd, list->GetId(2), destPoint++ );

              outpd->CopyData(inpd, list->GetId(0), destPoint++ );
              outpd->CopyData(inpd, list->GetId(1), destPoint++ );
              outpd->CopyData(inpd, list->GetId(3), destPoint++ );
              outpd->CopyData(inpd, list->GetId(2), destPoint++ );
            }
          }
        }
      }
    }
    // Fixed length extrusion or node scaled extrusion.
    else
    {
      vtkIdType n_out_pts = inpd->GetNumberOfTuples() * nLevels;
      outpd->CopyAllocate(inpd, n_out_pts);
      
      vtkIdType destPoint = 0;
      
      for (vtkIdType l = 0 ; l < nLevels ; l++)
      {
        vtkIdType nt = inpd->GetNumberOfTuples();
        
        for (vtkIdType srcPoint = 0 ; srcPoint < nt ; srcPoint++)
          outpd->CopyData(inpd, srcPoint, destPoint++);
      }
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
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;

    int dims[3] = {1,1,1};
    rgrid->GetDimensions(dims);

    if(dims[2] > 1)
    {
        EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
    }

    if(dims[1] == 1)
    {
        EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
    }

    debug5 << "Extrude rectilinear grid to rectilinear grid." << endl;

    dims[2] = atts.GetSteps() + 1;

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

// Helper function to extrude and insert a point into a VTK point list
#define extrude_points(type)                         \
{                                                    \
    type *pts = (type *) points->GetVoidPointer(0);  \
                                                     \
    double scalar = 1.0;                             \
                                                     \
    for(int l = 0; l < nLevels; ++l)                 \
    {                                                \
        type t = type(l) / type(nLevels-1);          \
                                                     \
        avtVector offset(atts.GetAxis());            \
        offset.normalize();                          \
        offset *= (atts.GetLength() * t);            \
                                                     \
        for(int n = 0; n < nNodes; ++n)              \
        {                                            \
            double pt[3];                            \
            inPoints->GetPoint(n, pt);               \
                                                     \
            if( nodeData )                           \
            {                                        \
              scalar = varArray->GetTuple1(n);       \
                                                     \
              if( minScalar > scalar )               \
                minScalar = scalar;                  \
                                                     \
              if( maxScalar < scalar )               \
                maxScalar = scalar;                  \
            }                                        \
                                                     \
            *pts++ = pt[0] + offset.x * scalar;      \
            *pts++ = pt[1] + offset.y * scalar;      \
            *pts++ = pt[2] + offset.z * scalar;      \
        }                                            \
    }                                                \
}

vtkPoints *
avtExtrudeFilter::CreateExtrudedPoints(vtkPoints *inPoints, int nLevels)
{
    vtkIdType nNodes = inPoints->GetNumberOfPoints();
    
    vtkPoints *points = vtkPoints::New(inPoints->GetDataType());
    points->SetNumberOfPoints(nNodes * nLevels);

    if(inPoints->GetDataType() == VTK_FLOAT)
    {
        extrude_points(float);
    }
    else if(inPoints->GetDataType() == VTK_DOUBLE)
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
// Returns : A new structured grid.
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

// Helper function to extrude and insert a point into a VTK point list
#define extrude_coords(type)                                   \
{                                                              \
    type *pts = (type *) points->GetVoidPointer(0);            \
                                                               \
    double scalar = 1.0;                                       \
                                                               \
    for(int k = 0; k < dims[2]; ++k)                           \
    {                                                          \
      type t = type(k) / type(dims[2]-1);                      \
      avtVector offset(atts.GetAxis());                        \
      offset.normalize();                                      \
      offset *= (atts.GetLength() * t);                        \
                                                               \
      vtkIdType cc = 0;                                        \
                                                               \
      for(int j = 0; j < dims[1]; ++j)                         \
      {                                                        \
        double y = rgrid->GetYCoordinates()->GetTuple1(j);     \
        for(int i = 0; i < dims[0]; ++i)                       \
        {                                                      \
          double x = rgrid->GetXCoordinates()->GetTuple1(i);   \
                                                               \
          if( nodeData )                                       \
          {                                                    \
            scalar = varArray->GetTuple1(cc++);                \
                                                               \
            if( minScalar > scalar )                           \
              minScalar = scalar;                              \
                                                               \
            if( maxScalar < scalar )                           \
              maxScalar = scalar;                              \
          }                                                    \
                                                               \
          *pts++ = x + offset.x * scalar;                      \
          *pts++ = y + offset.y * scalar;                      \
          *pts++ = z + offset.z * scalar;                      \
        }                                                      \
      }                                                        \
    }                                                          \
 }

vtkDataSet *
avtExtrudeFilter::ExtrudeToStructuredGrid(vtkDataSet *in_ds)
{
    int dims[3] = {1,1,1};
    vtkPoints *points = NULL;

    // Assemble the coordinates differently according to the input mesh type.
    if(in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) in_ds;
        sgrid->GetDimensions(dims);

        if(dims[2] > 1)
        {
            EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
        }

        debug5 << "Extrude structured grid to structured grid." << endl;

        dims[2] = atts.GetSteps()+1;

        if( atts.GetByVariable() )
        {
          vtkIdType nNodes = sgrid->GetPoints()->GetNumberOfPoints();
          vtkIdType nTuples = varArray->GetNumberOfTuples();

          if( nNodes != nTuples )
            EXCEPTION1(ImproperUseException,
                       "The number of scalar values does match the number of points.");
        }
        
        points = CreateExtrudedPoints(sgrid->GetPoints(), dims[2]);
    }
    else if(in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) in_ds;
        rgrid->GetDimensions(dims);

        if(dims[2] > 1)
        {
            EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
        }

        if(dims[1] == 1)
        {
            EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
        }

        if( atts.GetByVariable() )
        {
          vtkIdType nNodes = dims[0] * dims[1];
          vtkIdType nTuples = varArray->GetNumberOfTuples();

          if( nNodes != nTuples )
            EXCEPTION1(ImproperUseException,
                       "The number of scalar values does match the number of points.");
        }
        
        debug5 << "Extrude rectilinear grid to structured grid." << endl;

        double z;
        if( dims[2] == 1 )
          z = rgrid->GetZCoordinates()->GetTuple1(0);
        else
          z = 0;

        dims[2] = atts.GetSteps()+1;

        points = vtkVisItUtility::NewPoints(rgrid);
        points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);

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
    out_ds->SetDimensions(dims);
    out_ds->SetPoints(points);
    points->Delete();

    // Copy variables
    CopyVariables(in_ds, out_ds, dims[2]);

    return out_ds;
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeToUnstructuredGrid
//
// Purpose: 
//   Extrude unstructured grids and polydata into a new unstructured grid.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new unstructured grid.
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
avtExtrudeFilter::ExtrudeToUnstructuredGrid(vtkPointSet *in_ds)
{
    int nSteps = atts.GetSteps();

    vtkIdType nNodes = in_ds->GetNumberOfPoints();
    vtkIdType nCells = in_ds->GetNumberOfCells();
    
    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nNodes != nTuples )
          EXCEPTION1(ImproperUseException,
                     "The number of scalar values does match the number of points.");
    }
    
    debug5 << "Extrude unstructured grid to unstructured grid." << endl;

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkIdType n_out_cells = nCells * nSteps;
    ugrid->Allocate(n_out_cells * 8);
    
    vtkPoints *points = CreateExtrudedPoints(in_ds->GetPoints(), nSteps+1);
    ugrid->SetPoints(points);
    points->Delete();

    // Create the extruded connectivity
    int *cellReplication = NULL;

    for(int s = 0; s < nSteps; ++s)
    {
        for (vtkIdType cellid = 0 ; cellid < nCells ; cellid++)
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
            vtkIdType offset = s * nNodes;

            if (c == VTK_VERTEX)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(0) + offset + nNodes;
                ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else if(c == VTK_LINE)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(1) + offset + nNodes;
                verts[3] = list->GetId(0) + offset + nNodes;
                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
            else if (c == VTK_POLY_LINE)
            {
                // If we're seeing a polyline for the first time then we need to
                // keep track of cell replication (a count of how many cells this
                // cell was broken into) so we can copy the cell-centered variables 
                // properly.
                if(s == 0 && cellReplication == NULL)
                {
                    cellReplication = new int[nCells];
                    for(int r = 0; r < nCells; ++r)
                        cellReplication[r] = (r < cellid) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
                {
                    verts[0] = list->GetId(p)   + offset;
                    verts[1] = list->GetId(p+1) + offset;
                    verts[2] = list->GetId(p+1) + offset + nNodes;
                    verts[3] = list->GetId(p)   + offset + nNodes;
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);

                    if(s == 0)
                        cellReplication[cellid]++;
                }

                if(s == 0)
                    cellReplication[cellid]--;
            }
            else if (c == VTK_TRIANGLE)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(2) + offset;
                verts[3] = list->GetId(0) + offset + nNodes;
                verts[4] = list->GetId(1) + offset + nNodes;
                verts[5] = list->GetId(2) + offset + nNodes;
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            }
            else if(c == VTK_QUAD)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(2) + offset;
                verts[3] = list->GetId(3) + offset;
                verts[4] = list->GetId(0) + offset + nNodes;
                verts[5] = list->GetId(1) + offset + nNodes;
                verts[6] = list->GetId(2) + offset + nNodes;
                verts[7] = list->GetId(3) + offset + nNodes;
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else if(c == VTK_PIXEL)
            {
                verts[0] = list->GetId(0) + offset;
                verts[1] = list->GetId(1) + offset;
                verts[2] = list->GetId(3) + offset;
                verts[3] = list->GetId(2) + offset;
                verts[4] = list->GetId(0) + offset + nNodes;
                verts[5] = list->GetId(1) + offset + nNodes;
                verts[6] = list->GetId(3) + offset + nNodes;
                verts[7] = list->GetId(2) + offset + nNodes;
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }

            if(s == 0 && cellReplication != NULL)
                cellReplication[cellid]++;
        }
    }

    // Copy the variables
    CopyVariables(in_ds, ugrid, nSteps+1, cellReplication);

    delete [] cellReplication;

    return ugrid;
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeToStructuredGrid
//
// Purpose: 
//   Extrude a rectilinear grid into a new unstructured grid using
//   a cell based scalar value.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new structured grid.
//
// Note:
//
// Programmer: Allen Sanderson
// Creation:   Thu Sept 14 2018
//
// Modifications:
//   
// ****************************************************************************

// Helper function to extrude and insert a point into a VTK vertex list
#define extrude_2Dpoint( ix, iy, id_out, offset )                      \
{                                                                      \
  pt[0] = in_ds->GetXCoordinates()->GetTuple1(ix) + offset.x * scalar; \
  pt[1] = in_ds->GetYCoordinates()->GetTuple1(iy) + offset.y * scalar; \
  pt[2] =                                      z0 + offset.z * scalar; \
                                                                       \
  verts[id_out] = points->InsertNextPoint( pt[0], pt[1], pt[2] );      \
}

vtkDataSet *
avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid(vtkRectilinearGrid *in_ds)
{           
    int nSteps = atts.GetSteps();

    int dims[3] = {1,1,1};
    in_ds->GetDimensions(dims);

    if(dims[2] > 1)
    {
      EXCEPTION1(ImproperUseException, "3D data cannot be extruded.");
    }

    if(dims[1] == 1)
    {
      EXCEPTION1(ImproperUseException, "Extruding curves is not implemented.");
    }

    vtkIdType nCells = (dims[0]-1) * (dims[1]-1);

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nCells != nTuples )
          EXCEPTION1(ImproperUseException,
                     "The number of scalar values does match the number of cells.");
    }
    
    debug5 << "Extrude rectilinear grid to unstructured grid." << endl;

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkIdType n_out_cells = nCells * nSteps;
    ugrid->Allocate(n_out_cells * 8);

    vtkPoints *points = vtkPoints::New(in_ds->GetYCoordinates()->GetDataType());

    double pt[3];
    double scalar;

    vtkIdType verts[8];

    double z0;
    if( dims[2] == 1 )
      z0 = in_ds->GetZCoordinates()->GetTuple1(0);
    else
      z0 = 0;

    // Create the extruded connectivity
    for(int s=0; s<nSteps; ++s)
    {
        double t0 = double(s  ) / double(nSteps);
        double t1 = double(s+1) / double(nSteps);

        avtVector axis(atts.GetAxis());
        axis.normalize();
        
        avtVector offset0 = axis * atts.GetLength() * t0;
        avtVector offset1 = axis * atts.GetLength() * t1;
            
        for(int j=0; j<dims[1]-1; ++j)
        {
          for(int i=0; i<dims[0]-1; ++i)
          {
            vtkIdType cellid = j * (dims[0]-1) + i;

            scalar = varArray->GetTuple1(cellid);
            
            if( minScalar > scalar )
              minScalar = scalar;

            if( maxScalar < scalar )
              maxScalar = scalar;

            extrude_2Dpoint( i+0, j+0, 0, offset0 );
            extrude_2Dpoint( i+1, j+0, 1, offset0 );
            extrude_2Dpoint( i+1, j+1, 2, offset0 );
            extrude_2Dpoint( i+0, j+1, 3, offset0 );
            
            extrude_2Dpoint( i+0, j+0, 4, offset1 );
            extrude_2Dpoint( i+1, j+0, 5, offset1 );
            extrude_2Dpoint( i+1, j+1, 6, offset1 );
            extrude_2Dpoint( i+0, j+1, 7, offset1 );

            ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
          }
        }
    }
    
    // Assemble the mesh from the points.
    ugrid->SetPoints(points);
    points->Delete();

    // Copy the variables
    CopyVariables(in_ds, ugrid, nSteps+1);

    return ugrid;
}

// ****************************************************************************
// Method: avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid
//
// Purpose: 
//   Extrude a structured or an unstructured grids and polydata into
//    a new unstructured grid using a cell based scalar value.
//
// Arguments:
//   in_ds : The input dataset.
//
// Returns : A new unstructured grid.
//
// Note:
//
// Programmer: Allen Sanderson
// Creation:   Thu Sept 14 2018
//
// Modifications:
//   
// ****************************************************************************

// Helper function to extrude and insert a point into a VTK vert list
#define extrude_point( id_in, id_out, offset )                    \
{                                                                 \
  in_ds->GetPoint(list->GetId(id_in), pt );                       \
                                                                  \
  pt[0] += offset.x * scalar;                                     \
  pt[1] += offset.y * scalar;                                     \
  pt[2] += offset.z * scalar;                                     \
                                                                  \
  verts[id_out] = points->InsertNextPoint( pt[0], pt[1], pt[2] ); \
}

vtkDataSet *
avtExtrudeFilter::ExtrudeCellVariableToUnstructuredGrid(vtkPointSet *in_ds)
{
    int nSteps = atts.GetSteps();

    vtkIdType nCells = in_ds->GetNumberOfCells();

    if( atts.GetByVariable() )
    {
        vtkIdType nTuples = varArray->GetNumberOfTuples();

        if( nCells != nTuples )
          EXCEPTION1(ImproperUseException,
                     "The number of scalar values does match the number of cells.");
    }
    
    debug5 << "Extrude structured/unstructured grid to unstructured grid." << endl;

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkIdType n_out_cells = nCells * nSteps;
    ugrid->Allocate(n_out_cells * 8);
    
    vtkPoints *points = vtkPoints::New(in_ds->GetPoints()->GetDataType());

    // Create the extruded connectivity
    int *cellReplication = NULL;

    double pt[3];
    double scalar;
    
    for(int s=0; s<nSteps; ++s)
    {
        double t0 = double(s  ) / double(nSteps);
        double t1 = double(s+1) / double(nSteps);

        avtVector axis(atts.GetAxis());
        axis.normalize();
        
        avtVector offset0 = axis * atts.GetLength() * t0;
        avtVector offset1 = axis * atts.GetLength() * t1;
        
        for (vtkIdType cellid=0; cellid<nCells; ++cellid)
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

            scalar = varArray->GetTuple1(cellid);

            if( minScalar > scalar )
              minScalar = scalar;

            if( maxScalar < scalar )
              maxScalar = scalar;

            vtkIdList *list = cell->GetPointIds();
            vtkIdType verts[8];

            if (c == VTK_VERTEX)
            {
              extrude_point( 0, 0, offset0 );
              extrude_point( 0, 1, offset1 );

              ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else if(c == VTK_LINE)
            {
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 1, 2, offset1 );
              extrude_point( 0, 3, offset1 );
              
              ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
            else if (c == VTK_POLY_LINE)
            {
                // If we're seeing a polyline for the first time then
                // we need to keep track of cell replication (a count
                // of how many cells this cell was broken into) so we
                // can copy the cell-centered variables properly.
                if(s == 0 && cellReplication == NULL)
                {
                  cellReplication = new int[nCells];
                  for(int r = 0; r < nCells; ++r)
                    cellReplication[r] = (r < cellid) ? 1 : 0;
                }

                for(int p = 0; p < list->GetNumberOfIds()-1; ++p)
                {
                  extrude_point( p,   0, offset0 );
                  extrude_point( p+1, 1, offset0 );
                  extrude_point( p+1, 2, offset1 );
                  extrude_point( p,   3, offset1 );
              
                  if(s == 0)
                    cellReplication[cellid]++;
                }

                if(s == 0)
                    cellReplication[cellid]--;
            }
            else if (c == VTK_TRIANGLE)
            {
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 2, 2, offset0 );
              extrude_point( 0, 3, offset1 );
              extrude_point( 1, 4, offset1 );
              extrude_point( 2, 5, offset1 );
              
              ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            }
            else if(c == VTK_QUAD)
            {
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 2, 2, offset0 );
              extrude_point( 3, 3, offset0 );
              extrude_point( 0, 4, offset1 );
              extrude_point( 1, 5, offset1 );
              extrude_point( 2, 6, offset1 );
              extrude_point( 3, 7, offset1 );

              ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else if(c == VTK_PIXEL)
            {
              extrude_point( 0, 0, offset0 );
              extrude_point( 1, 1, offset0 );
              extrude_point( 3, 2, offset0 );
              extrude_point( 2, 3, offset0 );
              extrude_point( 0, 4, offset1 );
              extrude_point( 1, 5, offset1 );
              extrude_point( 3, 6, offset1 );
              extrude_point( 2, 7, offset1 );

              ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }

            if(s == 0 && cellReplication != NULL)
                cellReplication[cellid]++;
        }
    }

    // Assemble the mesh from the points.
    ugrid->SetPoints(points);
    points->Delete();

    // Copy the variables
    CopyVariables(in_ds, ugrid, nSteps+1, cellReplication);

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

    dbounds[0] = eMIN(dbounds[0], dbounds[0] + offset.x * minScalar);
    dbounds[1] = eMAX(dbounds[1], dbounds[1] + offset.x * maxScalar);
    dbounds[2] = eMIN(dbounds[2], dbounds[2] + offset.y * minScalar);
    dbounds[3] = eMAX(dbounds[3], dbounds[3] + offset.y * maxScalar);
    dbounds[4] = eMIN(dbounds[4], dbounds[4] + offset.z * minScalar);
    dbounds[5] = eMAX(dbounds[5], dbounds[5] + offset.z * maxScalar);
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
        outAtts.SetSpatialDimension(3);
    else
        outAtts.SetSpatialDimension(inAtts.GetSpatialDimension()+1);
    
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
