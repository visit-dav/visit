// ************************************************************************* //
//                             vtkVisItUtility.C                             //
// ************************************************************************* //

#include <vtkVisItUtility.h>

#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>


// ****************************************************************************
//  Function: GetPoints
//
//  Purpose:
//      A routine that will create a vtkPoints object from a dataset.  If the
//      dataset already has a points object, it will use that.
//
//  Arguments:
//      inDS    The dataset.
//
//  Returns:    A vtkPoints object.  The calling function must Delete this.
//
//  Programmer: Hank Childs
//  Creation:   August 18, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Jan  3 14:27:51 PST 2003
//    Account for point sets that don't have valid points.
//
// ****************************************************************************
 
vtkPoints *
vtkVisItUtility::GetPoints(vtkDataSet *inDS)
{
    int   i, j, k;
    vtkPoints *pts = NULL;
 
    int type = inDS->GetDataObjectType();
    if (type == VTK_POLY_DATA || type == VTK_UNSTRUCTURED_GRID ||
        type == VTK_STRUCTURED_GRID)
    {
        //
        // This dataset already has a vtkPoints object we can use.
        //
        vtkPointSet *pt_ds = (vtkPointSet *) inDS;
        pts = pt_ds->GetPoints();
        if (pts != NULL)
            pts->Register(NULL); // We are going to remove this later.
        else
            pts = vtkPoints::New();
    }
    else if (type == VTK_RECTILINEAR_GRID)
    {
        //
        // We will need to construct a vtkPoints object.
        //
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) inDS;
        vtkDataArray *xc = rgrid->GetXCoordinates();
        int nx = xc->GetNumberOfTuples();
        vtkDataArray *yc = rgrid->GetYCoordinates();
        int ny = yc->GetNumberOfTuples();
        vtkDataArray *zc = rgrid->GetZCoordinates();
        int nz = zc->GetNumberOfTuples();
        float *x = new float[nx];
        for (i = 0 ; i < nx ; i++)
        {
            x[i] = xc->GetComponent(i, 0);
        }
        float *y = new float[ny];
        for (i = 0 ; i < ny ; i++)
        {
            y[i] = yc->GetComponent(i, 0);
        }
        float *z = new float[nz];
        for (i = 0 ; i < nz ; i++)
        {
            z[i] = zc->GetComponent(i, 0);
        }
 
        pts = vtkPoints::New();
        pts->SetNumberOfPoints(nx*ny*nz);
        float *p = (float *) pts->GetVoidPointer(0);
        for (k = 0 ; k < nz ; k++)
        {
            for (j = 0 ; j < ny ; j++)
            {
                for (i = 0 ; i < nx ; i++)
                {
                    p[0] = x[i];
                    p[1] = y[j];
                    p[2] = z[k];
                    p += 3;
                }
            }
        }
 
        delete [] x;
        delete [] y;
        delete [] z;
    }
 
    return pts;
}


// ****************************************************************************
//  Function: GetLogicalIndices
//
//  Purpose:
//      A routine that returns the logical indices for a given point or cell id
//      within a structured dataset.   
//
//  Arguments:
//      ds      The dataset.
//      forCell Indicates whether the ID is for a cell (true) or a point (false). 
//      ID      The id of the point or the cell. 
//      ijk     A place to store the logical indices (all -1 if ds is not
//              a rectilinear or structured grid). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr 16 16:25:18 PDT 2003  
//    Improved logic to make it work correctly for cells and points.
//
// ****************************************************************************

void
vtkVisItUtility::GetLogicalIndices(vtkDataSet *ds, const bool forCell, const int ID, 
                                   int ijk[3])
{ 
    int dimX, dimY, dims[3], base[3] = {0, 0, 0};

    int type = ds->GetDataObjectType();
    if (type == VTK_STRUCTURED_GRID)
    {
        ((vtkStructuredGrid*)ds)->GetDimensions(dims);
    }
    else if (type == VTK_RECTILINEAR_GRID)
    {
        ((vtkRectilinearGrid*)ds)->GetDimensions(dims);
    }
    else 
    {
        ijk[0] = ijk[1] = ijk[2] = -1; 
        return; 
    }

    vtkIntArray *realDims = (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");

    if (forCell)
    {
        dimX = (dims[0]-1 > 0 ? dims[0]-1 : 1);
        dimY = (dims[1]-1 > 0 ? dims[1]-1 : 1);
        if (realDims)
        {
            base[0] = realDims->GetValue(0);
            base[1] = realDims->GetValue(2);
            base[2] = realDims->GetValue(4);
        }
    }
    else
    {
        dimX = (dims[0] == 0 ? 1 : dims[0]);
        dimY = (dims[1] == 0 ? 1 : dims[1]);
        if (realDims) 
        {
            base[0] = realDims->GetValue(0)-1;
            base[1] = realDims->GetValue(2)-1;
            base[2] = realDims->GetValue(4)-1;
        }
    }

    ijk[0] = (ID % dimX)          - base[0];
    ijk[1] = ((ID / dimX) % dimY) - base[1];
    ijk[2] = (ID / (dimX * dimY)) - base[2];
}



// ****************************************************************************
//  Function: CalculateRealCellID
//
//  Purpose:
//      A routine that calculates a 'real' cellId from structured data.
//      That is, the cell ID in the original non-ghost-zone data, which 
//      corresponds to the given cellId. 
//
//  Arguments:
//      cellId  The cellId in the ds.
//      ds      The dataset.
//
//  Returns:
//      If the passed dataset is structured, the "realcellId" calculated from 
//      non-ghost dimensions.  Otherwise, simply returns the passed cellId.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 16, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 18 14:01:41 PDT 2003 
//    Fix calculation of c.
//
// ****************************************************************************

int
vtkVisItUtility::CalculateRealCellID(const int cellId, vtkDataSet *ds)
{
    int retVal = cellId;
    int type = ds->GetDataObjectType();

    if (type == VTK_STRUCTURED_GRID ||
        type == VTK_RECTILINEAR_GRID) 
    {
        vtkIntArray *realDims = 
            (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
        if (realDims != NULL)
        {
            int IJK[3] = { -1, -1, -1};
            GetLogicalIndices(ds, true, cellId, IJK);
            int nCellsI, nCellsJ; 
            nCellsI = realDims->GetValue(1)- realDims->GetValue(0);
            nCellsJ = realDims->GetValue(3)- realDims->GetValue(2);
            int c = IJK[0] +
                    IJK[1] * nCellsI +  
                    IJK[2] * nCellsI *nCellsJ;
             retVal = c;
        }
    }
    return retVal;
}


// ****************************************************************************
//  Function: ComputeStructuredCoordinates
//
//  Purpose:
//    Computes the structured coordinates in a rectilinear grid for a point 
//    x[3]. The cell is specified by the array ijk[3]. 
//
//  Arguments:
//    rgrid  The rectilinear grid. 
//    x      The poit.
//    ijk    A place to store the structured coordinates. 
//
//  Returns:
//    0 if the point x is outside of the grid, and a 1 if inside the grid. 
//
//  Notes:  This method is taken from vtkRectilinearGrid.  It is modified
//          to allow x to line on the boundary of the grid, and does not
//          compute paramatric coordinates which are not necessary for our 
//          purposes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2003 
//
// ****************************************************************************


int
vtkVisItUtility::ComputeStructuredCoordinates(vtkRectilinearGrid *rgrid, 
                                              float x[3], int ijk[3])
{
  int i, j;
  float xPrev, xNext, tmp;
  vtkDataArray *scalars[3];
 
  scalars[0] = rgrid->GetXCoordinates();
  scalars[1] = rgrid->GetYCoordinates();
  scalars[2] = rgrid->GetZCoordinates();
  //
  // Find locations in x-y-z direction
  //
  ijk[0] = ijk[1] = ijk[2] = 0;
 
  for ( j=0; j < 3; j++ )
    {
    xPrev = scalars[j]->GetComponent(0, 0);
    xNext = scalars[j]->GetComponent(scalars[j]->GetNumberOfTuples()-1, 0);
    if (xNext < xPrev)
      {
      tmp = xNext;
      xNext = xPrev;
      xPrev = tmp;
      }
    if ( x[j] < xPrev || x[j] > xNext )
      {
      return 0;
      }
 
    for (i=1; i < scalars[j]->GetNumberOfTuples(); i++)
      {
      xNext = scalars[j]->GetComponent(i, 0);
      if ( x[j] >= xPrev && x[j] < xNext )
        {
        ijk[j] = i - 1;
        break;
        }
 
      else if ( x[j] == xNext )
        {
        ijk[j] = i - 1;
        break;
        }
      xPrev = xNext;
      }
    }
 
  return 1;
}
