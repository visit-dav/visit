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
//                             vtkVisItUtility.C                             //
// ************************************************************************* //

#include <vtkVisItUtility.h>

#include <float.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkHexahedron.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkShortArray.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItPointLocator.h>
#include <vtkDataSetWriter.h>


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
                                              double x[3], int ijk[3])
{
  int i, j;
  double xPrev, xNext, tmp;
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
vtkVisItUtility::FindCell(vtkDataSet *ds, double x[3])
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
        double pcoords[3], *weights = new double[8], diagLen, tol;
        double closestPoint[3], dist2;
        vtkIdList *cellIds, *ptIds;
   
        vtkVisItPointLocator *locator = vtkVisItPointLocator::New();
        locator->SetDataSet(ds);
        locator->SetIgnoreDisconnectedPoints(1);
        locator->BuildLocator();
 
        diagLen = ds->GetLength();
        if (nCells != 0)
            tol = diagLen / (double) nCells;
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
       
        double minDist2 = FLT_MAX;
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
vtkVisItUtility::GetCellCenter(vtkCell* cell, double center[3])
{
    double parametricCenter[3] = {0., 0., 0.};
    double coord[3] = {0., 0., 0.};
    int subId = -1;
    if (cell->GetNumberOfPoints() <= 27)
    {
        double weights[27];
        subId = cell->GetParametricCenter(parametricCenter);
        cell->EvaluateLocation(subId, parametricCenter, coord, weights);
    }
    else
    {
        double *weights = new double[cell->GetNumberOfPoints()];
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


// ****************************************************************************
//  Function: CellContainsPoint
//
//  Purpose:
//      Tests whether or not a cell contains a point.  Does this by testing
//      the side the point lies on each face of the cell.
//
//  Programmer: Hank Childs
//  Creation:   March 18, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Jan  5 11:01:40 PST 2007
//    Add support for degenerate hexes (faces inside out).
//
// ****************************************************************************

bool
vtkVisItUtility::CellContainsPoint(vtkCell *cell, const double *pt)
{
    int   i;

    int cellType = cell->GetCellType();
    if (cellType == VTK_HEXAHEDRON)
    {
        vtkHexahedron *hex = (vtkHexahedron *) cell;
        vtkPoints *pts = hex->GetPoints();
        // vtkCell sets its points object data type to double. 
        double *pts_ptr = (double *) pts->GetVoidPointer(0);
        static int faces[6][4] = { {0,4,7,3}, {1,2,6,5},
                           {0,1,5,4}, {3,7,6,2},
                           {0,3,2,1}, {4,5,6,7} };

        double center[3] = { 0., 0., 0. };
        for (i = 0 ; i < 8 ; i++)
        {
            center[0] += pts_ptr[3*i];
            center[1] += pts_ptr[3*i+1];
            center[2] += pts_ptr[3*i+2];
        }
        center[0] /= 8.;
        center[1] /= 8.;
        center[2] /= 8.;

        for (i = 0 ; i < 6 ; i++)
        {
            double dir1[3], dir2[3];
            int idx0 = faces[i][0];
            int idx1 = faces[i][1];
            int idx2 = faces[i][3];
            dir1[0] = pts_ptr[3*idx1] - pts_ptr[3*idx0];
            dir1[1] = pts_ptr[3*idx1+1] - pts_ptr[3*idx0+1];
            dir1[2] = pts_ptr[3*idx1+2] - pts_ptr[3*idx0+2];
            dir2[0] = pts_ptr[3*idx0] - pts_ptr[3*idx2];
            dir2[1] = pts_ptr[3*idx0+1] - pts_ptr[3*idx2+1];
            dir2[2] = pts_ptr[3*idx0+2] - pts_ptr[3*idx2+2];
            double cross[3];
            cross[0] = dir1[1]*dir2[2] - dir1[2]*dir2[1];
            cross[1] = dir1[2]*dir2[0] - dir1[0]*dir2[2];
            cross[2] = dir1[0]*dir2[1] - dir1[1]*dir2[0];
            double origin[3];
            origin[0] = pts_ptr[3*idx0];
            origin[1] = pts_ptr[3*idx0+1];
            origin[2] = pts_ptr[3*idx0+2];

            //
            // The plane is of the form Ax + By + Cz - D = 0.
            //
            // Using the origin, we can calculate D:
            // D = A*origin[0] + B*origin[1] + C*origin[2]
            //
            // We want to know if 'pt' gives:
            // A*pt[0] + B*pt[1] + C*pt[2] - D >=? 0.
            //
            // We can substitute in D to get
            // A*(pt[0]-origin[0]) + B*(pt[1]-origin[1]) + C*(pt[2-origin[2])
            //    ?>= 0
            //
            double val1 = cross[0]*(pt[0] - origin[0])
                       + cross[1]*(pt[1] - origin[1])
                       + cross[2]*(pt[2] - origin[2]);

            //
            // If the hexahedron is inside out, then val1 could be
            // negative, because the face orientation is wrong.
            // Find the sign for the cell center.
            //
            double val2 = cross[0]*(center[0] - origin[0])
                       + cross[1]*(center[1] - origin[1])
                       + cross[2]*(center[2] - origin[2]);

            // 
            // If the point in question (pt) and the center are on opposite
            // sides of the cell, then declare the point outside the cell.
            //
            if (val1*val2 < 0.)
                return false;
        }
        return true;
    }

    double closestPt[3];
    int subId;
    double pcoords[3];
    double dist2;
    double weights[100]; // MUST BE BIGGER THAN NPTS IN A CELL (ie 8).
    double non_const_pt[3];
    non_const_pt[0] = pt[0];
    non_const_pt[1] = pt[1];
    non_const_pt[2] = pt[2];
    return (cell->EvaluatePosition(non_const_pt, closestPt, subId,
                                  pcoords, dist2, weights) > 0);
}


// ****************************************************************************
//  Function: WriteDataSet
//
//  Purpose:
//     Writes out a vtk dataset.
//
//  Arguments:
//      ds      The dataset to write.
//      fname   The name of the output file.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2006
//
// ****************************************************************************

void
vtkVisItUtility::WriteDataSet(vtkDataSet *ds, const char *fname)
{
    vtkDataSetWriter *writer = vtkDataSetWriter::New();
    writer->SetInput(ds);
    writer->SetFileName(fname);
    writer->SetFileTypeToASCII();
    writer->Write();
    writer->Delete();
}


// ****************************************************************************
//  Function: Create1DRGrid
//
//  Purpose:
//    Creates a 1-dimensional vtkRectilinearGrid.
//
//  Arguments:
//    nXCoords  The number of X coordinates in the grid.
//    type      The data type for the coordinate arrays. 
//
//  Returns:
//    A 1-dimenional vtkRectilinearGrid
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2006
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 14 15:02:11 PDT 2008
//    Expanded the data types supported.
//
// ****************************************************************************

vtkRectilinearGrid * 
vtkVisItUtility::Create1DRGrid(int nXCoords, int type)
{
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(nXCoords, 1, 1);
    vtkDataArray *yz = NULL;
    vtkDataArray *xc = NULL;
    if (type == VTK_FLOAT)
    {
        xc = vtkFloatArray::New();
        yz = vtkFloatArray::New();
    }
    else if (type == VTK_DOUBLE)
    {
        xc = vtkDoubleArray::New();
        yz = vtkDoubleArray::New();
    }
    else if (type == VTK_INT)
    {
        xc = vtkIntArray::New();
        yz = vtkIntArray::New();
    }
    else if (type == VTK_SHORT)
    {
        xc = vtkShortArray::New();
        yz = vtkShortArray::New();
    }
    else if (type == VTK_CHAR)
    {
        xc = vtkCharArray::New();
        yz = vtkCharArray::New();
    }
    yz->SetNumberOfComponents(1);
    yz->SetNumberOfTuples(1);
    yz->SetTuple1(0, 0.);
    xc->SetNumberOfComponents(1);
    xc->SetNumberOfTuples(nXCoords);
    rgrid->SetXCoordinates(xc); 
    rgrid->SetYCoordinates(yz); 
    rgrid->SetZCoordinates(yz); 
    xc->Delete(); 
    yz->Delete(); 
    return rgrid;
}

// ****************************************************************************
//  Function: PointsEqual
//
//  Purpose:
//    Compares two 3d points for equality.
//
//  Arguments:
//    pt1       The first point.
//    pt2       The second point.
//
//  Returns:
//    True if points are equal, false otherwise. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   August 14, 2006
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 11 08:21:51 PDT 2009
//    Added optional tolerance argument.
// ****************************************************************************

bool 
vtkVisItUtility::PointsEqual(double pt1[3], double pt2[3], const double *_eps)
{
    double eps = 1e-6;
    if (_eps != NULL && *_eps < eps)
        eps = *_eps;
    bool e1 = false, e2 = false, e3 = false;
    if ((pt1[0] > pt2[0] - eps) && (pt1[0] < pt2[0]+eps))
        e1 = true;
    if ((pt1[1] > pt2[1] - eps) && (pt1[1] < pt2[1]+eps))
        e2 = true;
    if ((pt1[2] > pt2[2] - eps) && (pt1[2] < pt2[2]+eps))
        e3 = true;
    return e1 && e2 && e3;
}
