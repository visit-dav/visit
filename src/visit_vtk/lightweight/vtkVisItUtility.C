// ************************************************************************* //
//                             vtkVisItUtility.C                             //
// ************************************************************************* //

#include <vtkVisItUtility.h>

#include <float.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItPointLocator.h>


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
//      global  An optional flag indicating that the global indices should be
//              returned. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr 16 16:25:18 PDT 2003  
//    Improved logic to make it work correctly for cells and points.
//
//    Kathleen Bonnell, Wed Jun 25 13:27:59 PDT 2003 
//    Fixed logic for determining 'base' for points. 
//
//    Kathleen Bonnell, Tue Sep 16 13:33:30 PDT 2003 
//    Once again, redo logic for determining indices for a node, and allow
//    this method to work on a non-structured dataset if the array inidicating
//    original dimensions is present.
//
//    Kathleen Bonnell, Tue Dec  9 09:27:20 PST 2003 
//    If 'global' flag is set, use the "base_index" array to determine the
//    global 'ijk'. 
//
//    Kathleen Bonnell, Fri May 28 10:31:25 PDT 2004 
//    Added flag allowing adjustment for GhostZones.  Moved code that retrieves
//    dimensions to its own method. Account for 2D data differently than 3d.
// 
//    Kathleen Bonnell, Fri May 28 17:27:05 PDT 2004
//    Forgot return for bad dims, and test for invalid ijk (< 0). 
//
// ****************************************************************************

void
vtkVisItUtility::GetLogicalIndices(vtkDataSet *ds, const bool forCell, const int ID, 
                                   int ijk[3], const bool global,
                                   const bool adjustForGhosts)
{ 
    int dimX, dimY, dims[3], base[3] = {0, 0, 0};

    GetDimensions(ds, dims);
    if (dims[0] == -1 || dims[1] == -1 || dims[2] == -1)
    {
        ijk[0] = ijk[1] = ijk[2] = -1;
        return;
    }

    if (global)
    {
        vtkIntArray *bi = (vtkIntArray*)ds->GetFieldData()->GetArray("base_index");
        if (bi)
        {
            base[0] = bi->GetValue(0);
            base[1] = bi->GetValue(1);
            base[2] = bi->GetValue(2);
        }
    }

    if (adjustForGhosts)
    {
        vtkIntArray *realDims = 
            (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
        if (realDims)
        {
            base[0] -= realDims->GetValue(0);
            base[1] -= realDims->GetValue(2);
            base[2] -= realDims->GetValue(4);
        }
    }

    if (forCell)
    {
        dimX = (dims[0]-1 > 0 ? dims[0]-1 : 1);
        dimY = (dims[1]-1 > 0 ? dims[1]-1 : 1);
    }
    else
    {
        dimX = (dims[0] == 0 ? 1 : dims[0]);
        dimY = (dims[1] == 0 ? 1 : dims[1]);
    }

    if (dims[2] == 1)
    {
        ijk[0] = (ID % dimX) + base[0];
        ijk[1] = (ID / dimX) + base[1];
        ijk[2] = 0;
    }
    else 
    {
        ijk[0] = (ID % dimX)          + base[0];
        ijk[1] = ((ID / dimX) % dimY) + base[1];
        ijk[2] = (ID / (dimX * dimY)) + base[2];
    }

    ijk[0] = ijk[0] < 0 ? 0 : ijk[0];
    ijk[1] = ijk[1] < 0 ? 0 : ijk[1];
    ijk[2] = ijk[2] < 0 ? 0 : ijk[2];
}



// ****************************************************************************
//  Function: CalculateRealID
//
//  Purpose:
//      A routine that calculates a 'real' cellId from structured data.
//      That is, the cell ID in the original non-ghost-zone data, which 
//      corresponds to the given cellId. 
//
//  Arguments:
//      cellId  The cellId in the ds.
//      forCell True if a cell Id should be returned, false if  for point id. 
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
//    Kathleen Bonnell, Wed Jun 25 13:27:59 PDT 2003 
//    Renamed to reflect that this method can return node id or cell id, based
//    on the value of 'forCell' argument.
//
//    Kathleen Bonnell, Thu Sep 18 11:35:25 PDT 2003 
//    Allow this method to work on a non-structured dataset if the array 
//    inidicating original dimensions is present.
//
// ****************************************************************************

int
vtkVisItUtility::CalculateRealID(const int cellId, const bool forCell, vtkDataSet *ds)
{
    int retVal = cellId;
    int type = ds->GetDataObjectType();

    
    if (type == VTK_STRUCTURED_GRID ||
        type == VTK_RECTILINEAR_GRID ||
        ds->GetFieldData()->GetArray("vtkOriginalDimensions") != NULL) 
    {
        vtkIntArray *realDims = 
            (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
        if (realDims != NULL)
        {
            int IJK[3] = { -1, -1, -1};
            GetLogicalIndices(ds, forCell, cellId, IJK);
            int nElsI, nElsJ; 
            nElsI = realDims->GetValue(1)- realDims->GetValue(0);
            nElsJ = realDims->GetValue(3)- realDims->GetValue(2);
            if (!forCell)
            {
                nElsI += 1;
                nElsJ += 1;
            }
            retVal = IJK[0] +
                     IJK[1] * nElsI +  
                     IJK[2] * nElsI * nElsJ;
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


// ****************************************************************************
//  Function: FindCell
//
//  Purpose:
//    Searches the dataset for a cell containing the given point.
//
//  Arguments:
//    ds     The dataset to search. 
//    pt     The point.
//
//  Returns:
//    -1 if the point pt is outside of the ds, and the cell Id of the cell
//     containing the point otherwise. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 13, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb 18 10:03:21 PST 2004
//    Pulled code from vtkPointSet::FindCell, so that could be modified and
//    made more useful.
//
//    Kathleen Bonnell, Fri Apr 16 11:48:28 PDT 2004 
//    Use VisIt version of the point locator so that disconnected points can
//    be ignored.  In case there are physically adjacent but non-logically-
//    connected cells, find 8 closest points instead of 1.
//
//    Kathleen Bonnell, Wed Jul  7 15:02:03 PDT 2004 
//    Delete objects before early return, to prevent memory leaks.
//
// ****************************************************************************

int
vtkVisItUtility::FindCell(vtkDataSet *ds, float x[3])
{
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int ijk[3];
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid*)ds;
        if (ComputeStructuredCoordinates(rgrid, x, ijk) == 0)
            return -1;
        return rgrid->ComputeCellId(ijk);
    }
    else
    {
        // Pulled this from vtkPointSet::FindCell, because for
        // some of our data, their default 'MAXWALK' is too small.
        int nCells = ds->GetNumberOfCells();
        if (nCells == 0)
        {
            return -1;
        }

        vtkIdType ptId, cellId;
        vtkCell *cell;
        int walk, found = -1, subId;
        float pcoords[3], *weights = new float[8], diagLen, tol;
        float closestPoint[3], dist2;
        vtkIdList *cellIds, *ptIds;
   
        vtkVisItPointLocator *locator = vtkVisItPointLocator::New();
        locator->SetDataSet(ds);
        locator->SetIgnoreDisconnectedPoints(1);
        locator->BuildLocator();
 
        diagLen = ds->GetLength();
        if (nCells != 0)
            tol = diagLen / (float) nCells;
        else
            tol = 1e-6;

        //
        // Finding ONLY the single closest point won't work for this use-case 
        // if any adjacent cells share identical point coordinates with 
        // different point Ids.  (E.g. physically adjacent cells which are not
        // logically connected.) So find 8 closest-points and work from there.
        // 
        vtkIdList *closestPoints = vtkIdList::New();
        locator->FindClosestNPoints(8, x, closestPoints);
        if (closestPoints->GetNumberOfIds() == 0)
        {
            locator->Delete();
            closestPoints->Delete();
            delete [] weights;
            return -1;
        }
       
        float minDist2 = FLT_MAX;
        cellIds = vtkIdList::New();
        cellIds->Allocate(8, 100);
        ptIds = vtkIdList::New();
        ptIds->Allocate(8, 100);

        for (int z = 0; z < closestPoints->GetNumberOfIds() && found == -1; z++)
        {
            ptId = closestPoints->GetId(z);
            ds->GetPointCells(ptId, cellIds);
            if (cellIds->GetNumberOfIds() > 0)
            {
                cellId = cellIds->GetId(0);
                cell = ds->GetCell(cellId);
                int evaluate = cell->EvaluatePosition
                    (x, closestPoint, subId, pcoords, dist2, weights);

                if (evaluate == 1 && dist2 <= tol && dist2 < minDist2)
                {
                    found = cellId;
                    minDist2 = dist2;
                }
            }
            int MAXWALK = 50;
            if (found == -1 && cellIds->GetNumberOfIds() > 0)
            {
                for (walk = 0; walk < MAXWALK && minDist2 != 0. ; walk++)
                {
                    cell->CellBoundary(subId, pcoords, ptIds);
                    ds->GetCellNeighbors(cellId, ptIds, cellIds);
                    if (cellIds->GetNumberOfIds() > 0)
                    {
                        cellId = cellIds->GetId(0);
                        cell = ds->GetCell(cellId);
                    }
                    else 
                    {
                        break; // outside of data
                    }
                    if (cell)
                    {
                        int eval = cell->EvaluatePosition
                            (x, closestPoint, subId, pcoords, dist2, weights); 
                        if (eval == 1 && dist2 <= tol && dist2 < minDist2)
                        {
                            minDist2 = dist2;
                            found = cellId; 
                        }
                    }

                }
            }
        }

        delete [] weights;
        ptIds->Delete();
        cellIds->Delete();
        closestPoints->Delete();
        locator->Delete();
        return found;
    }
}


// ****************************************************************************
//  Function: GetDimensions
//
//  Purpose:
//      A routine that will return the dimensions of a structured dataset. 
//
//  Arguments:
//      ds      The dataset.
//      dims    A place to hold the dimensions.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 27, 2004 
//
//  Modifications:
//
// ****************************************************************************
 
void
vtkVisItUtility::GetDimensions(vtkDataSet *ds, int dims[3])
{ 
    dims[0] = dims[1] = dims[2] = -1;
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
        vtkIntArray *vtkDims = 
           (vtkIntArray*)ds->GetFieldData()->GetArray("vtkOriginalDimensions");
        if (vtkDims != NULL)
        {
            dims[0] = vtkDims->GetValue(0);
            dims[1] = vtkDims->GetValue(1);
            dims[2] = vtkDims->GetValue(2);
        }
    }
}


// ****************************************************************************
//  Function: CalculateGhostIdFromNonGhost
//
//  Purpose:
//    A routine that calculates a node or cell id in relation to the 
//    with-ghost-zone dataset, from an id that corresponds to the same
//    dataset without ghost-zones.
//
//  Arguments:
//    id    The cellId in the ds.
//      forCell True if a cell Id should be returned, false if  for point id. 
//      ds      The dataset.
//
//  Returns:
//    If the passed dataset is not structured, simply returns the passed id.
//    Otherwise returns the id corresponding to with-ghost-zone data.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 8, 2004 
//
//  Modifications:
//
// ****************************************************************************

int
vtkVisItUtility::CalculateGhostIdFromNonGhost(vtkDataSet *ds, const int id,
                                              const bool forCell)
{
    int retVal = id;
    int type = ds->GetDataObjectType();

    if (type == VTK_STRUCTURED_GRID ||
        type == VTK_RECTILINEAR_GRID ||
        ds->GetFieldData()->GetArray("vtkOriginalDimensions") != NULL) 
    {
        int dimX, dimY, dims[3]; 
        int ijk[3] = { -1, -1, -1};
        GetDimensions(ds, dims);
        if (dims[0] == -1 || dims[1] == -1 || dims[2] == -1)
        {
            return id;
        }
        vtkIntArray *realDims = 
            (vtkIntArray*)ds->GetFieldData()->GetArray("avtRealDims");
        if (realDims != NULL)
        {
            dimX = realDims->GetValue(1) - realDims->GetValue(0);
            dimY = realDims->GetValue(3) - realDims->GetValue(2);
            if (!forCell)
            {
                dimX++;
                dimY++;
            }
        }
        else 
        {
            if (forCell)
            {
                dimX = (dims[0]-1 > 0 ? dims[0]-1 : 1);
                dimY = (dims[1]-1 > 0 ? dims[1]-1 : 1);
            }
            else
            {
                dimX = (dims[0] == 0 ? 1 : dims[0]);
                dimY = (dims[1] == 0 ? 1 : dims[1]);
            }
        }
        if (dims[2] == 1)
        {
            ijk[0] = (id % dimX);
            ijk[1] = (id / dimX);
            ijk[2] = 0;
        }
        else 
        {
            ijk[0] = (id % dimX);          
            ijk[1] = ((id / dimX) % dimY);
            ijk[2] = (id / (dimX * dimY));
        }

        ijk[0] = ijk[0] < 0 ? 0 : ijk[0];
        ijk[1] = ijk[1] < 0 ? 0 : ijk[1];
        ijk[2] = ijk[2] < 0 ? 0 : ijk[2];

        if (realDims != NULL)
        {
            ijk[0] += realDims->GetValue(0);
            ijk[1] += realDims->GetValue(2);
            ijk[2] += realDims->GetValue(4);
        }

        int nElsI = dims[0];
        int nElsJ = dims[1];
        if (forCell)
        {
            nElsI -= 1;
            nElsJ -= 1;
        }
        retVal = ijk[0] +
                 ijk[1] * nElsI +  
                 ijk[2] * nElsI * nElsJ;
    }
    return retVal;
}

int
vtkVisItUtility::NodeGhostIdFromNonGhost(vtkDataSet *ds, const int node)
{
    return CalculateGhostIdFromNonGhost(ds, node, false);
}

int
vtkVisItUtility::ZoneGhostIdFromNonGhost(vtkDataSet *ds, const int zone)
{
    return CalculateGhostIdFromNonGhost(ds, zone, true);
}


// ****************************************************************************
//  Function: GetCellCenter
//
//  Purpose:
//    Calculates the coordinates of the cell center.
//
//  Arguments:
//    cell      The cell.
//    center    Storage for the coordinates.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 7, 2004
//
//  Modifications:
//
// ****************************************************************************

void
vtkVisItUtility::GetCellCenter(vtkCell* cell, float center[3])
{
    float parametricCenter[3] = {0., 0., 0.};
    float coord[3] = {0., 0., 0.};
    int subId = -1;
    if (cell->GetNumberOfPoints() <= 27)
    {
        float weights[27];
        subId = cell->GetParametricCenter(parametricCenter);
        cell->EvaluateLocation(subId, parametricCenter, coord, weights);
    }
    else
    {
        float *weights = new float[cell->GetNumberOfPoints()];
        subId = cell->GetParametricCenter(parametricCenter);
        cell->EvaluateLocation(subId, parametricCenter, coord, weights);
        delete [] weights;
    }
    center[0] = coord[0];
    center[1] = coord[1];
    center[2] = coord[2];
}

 
// ****************************************************************************
//  Function: GetLocalElementForGlobal
//
//  Purpose:
//    Given a globalElement id (zonal or nodal), determine the local element
//    id whose global id matches. 
//
//  Arguments:
//    ds             The dataset to be queried.
//    globalElement  Storage for the coordinates.
//    forCell        True if this query is for zonal ids, false otherwise. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 13, 2004 
//
//  Modifications:
//
// ****************************************************************************

int
vtkVisItUtility::GetLocalElementForGlobal(vtkDataSet* ds, 
    const int globalElement, const bool forCell)
{
    int retVal = -1;
    vtkIntArray *globalIds = NULL; 
    if (forCell)
    {
        globalIds = vtkIntArray::SafeDownCast(
                ds->GetCellData()->GetArray("avtGlobalZoneNumbers"));
    }
    else 
    {
        globalIds = vtkIntArray::SafeDownCast(
                ds->GetPointData()->GetArray("avtGlobalNodeNumbers"));
    }

    if (globalIds)
    {
        int n = globalIds->GetNumberOfTuples();
        int *g = globalIds->GetPointer(0);
        for (int i = 0; i < n && retVal == -1; i++)
        {
            retVal = (g[i] == globalElement ? i : -1);
        }
    }
    return retVal; 
}


// ****************************************************************************
//  Function: ContainsMixedGhostZoneTypes
//
//  Purpose:
//    Searches the dataset for a cell containing the given point.
//
//  Arguments:
//    ds     The dataset to search. 
//
//  Returns:
//    false if the dataset contains no ghosts, or only AMR type ghosts, 
//    true otherwise
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 8, 2005 
//
// ****************************************************************************

bool       
vtkVisItUtility::ContainsMixedGhostZoneTypes(vtkDataSet *ds)
{
    bool mixed = false;
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    if (ghosts)
    {
        unsigned char *gz = (unsigned char*)ghosts->GetVoidPointer(0); 
        for (int i = 0; i < ghosts->GetNumberOfTuples() && !mixed; i++)
        {
            mixed = !((int)gz[i] == 0 || (int)gz[i] == 8);
        }
    }
    return mixed;
}
