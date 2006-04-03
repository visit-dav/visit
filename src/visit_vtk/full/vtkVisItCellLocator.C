/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItCellLocator.cxx,v $
  Language:  C++
  Date:      $Date: 2002/01/22 15:28:01 $
  Version:   $Revision: 1.72 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItCellLocator.h"

#include <vtkUnsignedIntArray.h>
#include <vtkBox.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <vtkHexahedron.h>
#include <vtkIdList.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPixel.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkPolyLine.h>
#include <vtkPolyVertex.h>
#include <vtkPyramid.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkTriangle.h>
#include <vtkTriangleStrip.h>
#include <vtkVertex.h>
#include <vtkVoxel.h>
#include <vtkWedge.h>

#include <TimingsManager.h>
#include <vtkVisItUtility.h>



#include <stdlib.h>
#include <math.h>

vtkCxxRevisionMacro(vtkVisItCellLocator, "$Revision: 1.72 $");
vtkStandardNewMacro(vtkVisItCellLocator);

#define VTK_CELL_OUTSIDE 0
#define VTK_CELL_INSIDE 1

typedef vtkIdList *vtkIdListPtr;

class vtkNeighborCells
{
  public:
    vtkNeighborCells(const int sz, const int ext=1000)
      {this->P = vtkIntArray::New(); this->P->Allocate(3*sz,3*ext);};
    ~vtkNeighborCells(){this->P->Delete();}; 
    int GetNumberOfNeighbors() {return (this->P->GetMaxId()+1)/3;};
    void Reset() {this->P->Reset();};
    
    int *GetPoint(int i) {return this->P->GetPointer(3*i);};
    int InsertNextPoint(int *x);
    
  protected:
    vtkIntArray *P;
};

inline int vtkNeighborCells::InsertNextPoint(int *x) 
{
  int id = this->P->GetMaxId() + 3;
  this->P->InsertValue(id,x[2]);
  this->P->SetValue(id-2, x[0]);
  this->P->SetValue(id-1, x[1]);
  return id/3;
}

int
LineLineIsect(const double *, const double *, const double *, const double *, double *);

int
EdgeLineIsect(vtkCell *cell, const double *, const double *, double *);

// Construct with automatic computation of divisions, averaging
// 25 cells per bucket.
//
// Modificatons:
//   Kathleen Bonnell, Tue Jun  3 15:26:52 PDT 2003
//   Initialize MinCellLength.
//
//   Kathleen Bonnell, Wed Jun 18 18:27:18 PDT 2003 
//   Initialize triangle and quad. 
//
//   Kathleen Bonnell, Thu Nov  6 08:18:54 PST 2003 
//   Initialize UserBounds. 
//
vtkVisItCellLocator::vtkVisItCellLocator()
{
  this->NumberOfCellsPerBucket = 25;
  this->Tree = NULL;
  this->CellHasBeenVisited = NULL;
  this->QueryNumber = 0;
  this->H[0] = this->H[1] = this->H[2] = 1.0;
  this->NumberOfDivisions = 1;

  this->Buckets = new vtkNeighborCells(10, 10);
  this->CacheCellBounds = 0;
  this->CellBounds = NULL;
  this->IgnoreGhosts = 0;
  this->IgnoreLines = 0;

  this->OctantBounds[0] = 0.;
  this->OctantBounds[1] = 0.;
  this->OctantBounds[2] = 0.;
  this->OctantBounds[3] = 0.;
  this->OctantBounds[4] = 0.;
  this->OctantBounds[5] = 0.;
  this->UserBounds[0] = 0.;
  this->UserBounds[1] = 0.;
  this->UserBounds[2] = 0.;
  this->UserBounds[3] = 0.;
  this->UserBounds[4] = 0.;
  this->UserBounds[5] = 0.;
  this->userBoundsSet = false;
 
  this->MinCellLength = VTK_DOUBLE_MAX;

  this->triangle = vtkTriangle::New();
  this->quad = vtkQuad::New();

  this->TestCoPlanar = false;
}

//
// Modificatons:
//   Kathleen Bonnell, Wed Jun 18 18:27:18 PDT 2003 
//   Delete triangle and quad. 
//

vtkVisItCellLocator::~vtkVisItCellLocator()
{
  if (this->Buckets)
    {
    delete this->Buckets;
    this->Buckets = NULL;
    }
  
  this->FreeSearchStructure();
 
  if (this->CellHasBeenVisited)
    {
    delete [] this->CellHasBeenVisited;
    this->CellHasBeenVisited = NULL;
    }

  if (this->CellBounds)
    {
    delete [] this->CellBounds;
    this->CellBounds = NULL;
    }
  if (this->triangle)
    {
    this->triangle->Delete();
    this->triangle = NULL;
    }
  if (this->quad)
    {
    this->quad->Delete();
    this->quad = NULL;
    }
}

void vtkVisItCellLocator::FreeSearchStructure()
{
  vtkIdList *cellIds;
  int i;
  
  if ( this->Tree )
    {
    for (i=0; i<this->NumberOfOctants; i++)
      {
      cellIds = this->Tree[i];
      if (cellIds == (void *)VTK_CELL_INSIDE)
        {
        cellIds = 0;
        }
      if (cellIds)
        {
        cellIds->Delete();
        }
      }
    delete [] this->Tree;
    this->Tree = NULL;
    }
}

// Given an offset into the structure, the number of divisions in the octree,
// an i,j,k location in the octree; return the index (idx) into the structure.
// Method returns 1 is the specified i,j,k location is "outside" of the octree.
int vtkVisItCellLocator::GenerateIndex(int offset, int numDivs, int i, int j,
                                  int k, vtkIdType &idx)
{
  if ( i < 0 || i >= numDivs || 
       j < 0 || j >= numDivs || k < 0 || k >= numDivs )
    {
    return 1;
    }
  
  idx = offset + i + j*numDivs + k*numDivs*numDivs;
  
  return 0;
}

static
double
GetParametricDistance(vtkCell *cell, double pcoords[3]);

// Return intersection point (if any) of finite line with cells contained
// in cell locator.
int vtkVisItCellLocator::IntersectWithLine(double a0[3], double a1[3], double tol,
                                      double& t, double x[3], double pcoords[3],
                                      int &subId)
{
  vtkIdType cellId = -1;
  
  return this->IntersectWithLine( a0, a1, tol, t, x, pcoords,
                                  subId, cellId);
}

// Return intersection point (if any) AND the cell which was intersected by
// finite line
int vtkVisItCellLocator::IntersectWithLine(double a0[3], double a1[3], double tol,
                                      double& t, double x[3], double pcoords[3],
                                      int &subId, vtkIdType &cellId)
{
  vtkGenericCell *cell=vtkGenericCell::New();
  int returnVal;

  returnVal = this->IntersectWithLine( a0, a1, tol, t, x, pcoords, subId,
                                       cellId, cell);

  cell->Delete();
  return returnVal;
}
  
    
// Return intersection point (if any) AND the cell which was intersected by
// finite line
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003
//    Incorporate changes made to vtkCellLocator by Will Schroeder.
//    The changes fix a bug when dataset thickness is very near zero.
//
//    Kathleen Bonnell, Wed May  7 17:32:59 PDT 2003
//    Incorporate changes suggested by Will Schroeder in order to support
//    more accurate picking.  Call to static method GetParametricDistance
//    can be replaced by cell->GetParamatricDistance when we upgrade to the 
//    vtk version (> May 7, 2003) that supports this. 
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//

int vtkVisItCellLocator::IntersectWithLine(double a0[3], double a1[3], double tol,
                                      double& t, double x[3], double pcoords[3],
                                      int &subId, vtkIdType &cellId,
                                      vtkGenericCell *cell)
{
  double origin[3];
  double direction1[3];
  double direction2[3];
  double direction3[3];
  double hitPosition[3];
  double hitCellBoundsPosition[3], cellBounds[6];
  int hitCellBounds;
  double result;
  double bounds2[6];
  int i, leafStart, prod, loop;
  vtkIdType bestCellId = -1, cId, tempCellId;
  int idx;
  double tMax, dist[3];
  int npos[3];
  int pos[3];
  int bestDir, cellIsGhost;
  double stopDist, currDist;
  double deltaT, pDistance, minPDistance = 1.0e38;
  double length, maxLength = 0.0;
  vtkUnsignedCharArray *ghosts = 
    (vtkUnsignedCharArray *)this->DataSet->GetCellData()->GetArray("avtGhostZones");
  
  // convert the line into i,j,k coordinates
  tMax = 0.0;
  for (i=0; i < 3; i++) 
    {
    direction1[i] = a1[i] - a0[i];
    length = this->Bounds[2*i+1] - this->Bounds[2*i];
    if (length > maxLength)
      {
      maxLength = length;
      }
    origin[i] = (a0[i] - this->Bounds[2*i]) / length; 
    direction2[i] = direction1[i] / length; 

    bounds2[2*i]   = 0.0;
    bounds2[2*i+1] = 1.0;
    tMax += direction2[i]*direction2[i];
    }
  // create a parametric range around the tolerance
  deltaT = tol/maxLength;
 
  stopDist = tMax*this->NumberOfDivisions;

  for (i = 0; i < 3; i++) 
    {
        direction3[i] = direction2[i]/tMax;
    }

  if (vtkBox::IntersectBox(bounds2, origin, direction2, hitPosition, result))
    {
    // start walking through the octants
    prod = this->NumberOfDivisions*this->NumberOfDivisions;
    leafStart = this->NumberOfOctants - this->NumberOfDivisions*prod;
    bestCellId = -1;
    
    // Clear the array that indicates whether we have visited this cell.
    // The array is only cleared when the query number rolls over.  This
    // saves a number of calls to memset.
    this->QueryNumber++;
    if (this->QueryNumber == 0)
      {
      this->ClearCellHasBeenVisited();
      this->QueryNumber++;    // can't use 0 as a marker
      }
    
    // set up curr and stop dist
    currDist = 0;
    for (i = 0; i < 3; i++)
      {
      currDist += (hitPosition[i] - origin[i])*(hitPosition[i] - origin[i]);
      }
    currDist = sqrt(currDist)*this->NumberOfDivisions;
    
    // add one offset due to the problems around zero
    for (loop = 0; loop <3; loop++)
      {
      hitPosition[loop] = hitPosition[loop]*this->NumberOfDivisions + 1.0;
      pos[loop] = (int)hitPosition[loop];
      // Adjust right boundary condition: if we intersect from the top, right,
      // or back; then pos must be adjusted to a valid octant index 
      if (pos[loop] > this->NumberOfDivisions)
        {
        pos[loop] = this->NumberOfDivisions;
        }
      }
    
    idx = leafStart + pos[0] - 1 + (pos[1] - 1)*this->NumberOfDivisions 
      + (pos[2] - 1)*prod;
    while ((bestCellId < 0) && (pos[0] > 0) && (pos[1] > 0) && (pos[2] > 0) &&
      (pos[0] <= this->NumberOfDivisions) &&
      (pos[1] <= this->NumberOfDivisions) &&
      (pos[2] <= this->NumberOfDivisions) &&
      (currDist < stopDist))
      {
      if (this->Tree[idx])
        {
        this->ComputeOctantBounds(pos[0]-1, pos[1]-1, pos[2]-1);
        for (tMax = VTK_DOUBLE_MAX, tempCellId=0; 
        tempCellId < this->Tree[idx]->GetNumberOfIds(); tempCellId++) 
          {
          cId = this->Tree[idx]->GetId(tempCellId);
          if (this->CellHasBeenVisited[cId] != this->QueryNumber)
            {
            this->CellHasBeenVisited[cId] = this->QueryNumber;
            cellIsGhost = 0;
            if (ghosts && this->IgnoreGhosts)
              {
              cellIsGhost = (int)ghosts->GetComponent(cId, 0); 
              }
            if (!cellIsGhost)
              {
              hitCellBounds = 0;
            
              // check whether we intersect the cell bounds
              if (this->CacheCellBounds)
                {
                hitCellBounds = vtkBox::IntersectBox(this->CellBounds[cId],
                                                 a0, direction1,
                                                 hitCellBoundsPosition, result);
                }
              else 
                {
                this->DataSet->GetCellBounds(cId, cellBounds);
                hitCellBounds = vtkBox::IntersectBox(cellBounds,
                                                 a0, direction1,
                                                 hitCellBoundsPosition, result);
                }

              if (hitCellBounds)
                {
                // now, do the expensive GetCell call and the expensive
                // intersect with line call
                this->DataSet->GetCell(cId, cell);
            
                if (cell->IntersectWithLine(a0, a1, tol, t, x, pcoords, subId))
                  {
                  if (!this->IsInOctantBounds(x))
                   {
                   this->CellHasBeenVisited[cId] = 0; // mark the cell non-visited
                   }
                  else if (t < (tMax+deltaT)) // it might be close
                    {
                    pDistance = GetParametricDistance(cell, pcoords);
                    if (pDistance < minPDistance || 
                        (pDistance == minPDistance && t <tMax))
                      {
                      tMax = t;
                      bestCellId = cId;
                      minPDistance = pDistance;  
                      }
                    } // t <= tMax+tol
                  } // cell Isected line
                } // if (hitCellBounds)
              } // if !cellIsGhost
            } // if (!this->CellHasBeenVisited[cId])
          }
        }
      
      // move to the next octant
      tMax = VTK_DOUBLE_MAX;
      bestDir = 0;
      for (loop = 0; loop < 3; loop++)
        {
        if (direction3[loop] > 0)
          {
          npos[loop] = pos[loop] + 1;
          dist[loop] = (1.0 - hitPosition[loop] + pos[loop])/direction3[loop];
          if (dist[loop] == 0)
            {
            dist[loop] = 1.0/direction3[loop];
            }
          if (dist[loop] < 0)
            {
            dist[loop] = 0;
            }
          if (dist[loop] < tMax)
            {
            bestDir = loop;
            tMax = dist[loop];
            }
          }
        if (direction3[loop] < 0)
          {
          npos[loop] = pos[loop] - 1;
          dist[loop] = (pos[loop] - hitPosition[loop])/direction3[loop];
          if (dist[loop] == 0)
            {
            dist[loop] = -0.01/direction3[loop];
            }
          if (dist[loop] < 0)
            {
            dist[loop] = 0;
            }
          if (dist[loop] < tMax)
            {
            bestDir = loop;
            tMax = dist[loop];
            }
          }
        }
      // update our position
      for (loop = 0; loop < 3; loop++)
        {
        hitPosition[loop] += dist[bestDir]*direction3[loop];
        }
      currDist += dist[bestDir];
      // now make the move, find the smallest distance
      // only cross one boundry at a time
      pos[bestDir] = npos[bestDir];
      
      idx = leafStart + pos[0] - 1 + (pos[1]-1)*this->NumberOfDivisions + 
        (pos[2]-1)*prod;
      }
    }
    
  if (bestCellId >= 0)
    {

    this->DataSet->GetCell(bestCellId, cell);
    cell->IntersectWithLine(a0, a1, tol, t, x, pcoords, subId);
      
    // store the best cell id in the return "parameter"
    cellId = bestCellId;
    return 1;
    }
    
  return 0;
}


//
// Created by:  Kathleen Bonnell, June 18, 2003 
// Modified from above routine to use new cell intersection code that
// does not depend upon a tolerance.
//
// Modifications:
//   Kathleen Bonnell, Wed Sep 10 09:31:08 PDT 2003
//   Ensure that the 't' argument gets the correct value upon successful
//   intersection.
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
int vtkVisItCellLocator::IntersectWithLine(double a0[3], double a1[3], 
                                      double& t, double x[3], double pcoords[3],
                                      int &subId, vtkIdType &cellId)
{
  vtkGenericCell *cell = vtkGenericCell::New();

  double origin[3];
  double direction1[3];
  double direction2[3];
  double direction3[3];
  double hitPosition[3];
  double hitCellBoundsPosition[3], cellBounds[6];
  int hitCellBounds;
  double result;
  double bounds2[6];
  int i, leafStart, prod, loop;
  vtkIdType bestCellId = -1, cId;
  int idx;
  double tMax, dist[3];
  int npos[3];
  int pos[3];
  int bestDir, cellIsGhost;
  double stopDist, currDist;
  double length, maxLength = 0.0;
  double tempT, tempX[3], pc[3] = {0., 0., 0.};
  int tempId;
  vtkUnsignedCharArray *ghosts = 
    (vtkUnsignedCharArray *)this->DataSet->GetCellData()->GetArray("avtGhostZones");

  // convert the line into i,j,k coordinates
  tMax = 0.0;
  for (i=0; i < 3; i++) 
    {
    direction1[i] = a1[i] - a0[i];
    length = this->Bounds[2*i+1] - this->Bounds[2*i];
    if (length > maxLength)
      {
      maxLength = length;
      }
    origin[i] = (a0[i] - this->Bounds[2*i]) / length; 
    direction2[i] = direction1[i] / length; 

    bounds2[2*i]   = 0.0;
    bounds2[2*i+1] = 1.0;
    tMax += direction2[i]*direction2[i];
    }
  stopDist = tMax*this->NumberOfDivisions;

  for (i = 0; i < 3; i++) 
    {
        direction3[i] = direction2[i]/tMax;
    }

  if (vtkBox::IntersectBox(bounds2, origin, direction2, hitPosition, result))
    {
    // start walking through the octants
    prod = this->NumberOfDivisions*this->NumberOfDivisions;
    leafStart = this->NumberOfOctants - this->NumberOfDivisions*prod;
    bestCellId = -1;
    
    // Clear the array that indicates whether we have visited this cell.
    // The array is only cleared when the query number rolls over.  This
    // saves a number of calls to memset.
    this->QueryNumber++;
    if (this->QueryNumber == 0)
      {
      this->ClearCellHasBeenVisited();
      this->QueryNumber++;    // can't use 0 as a marker
      }
    
    // set up curr and stop dist
    currDist = 0;
    for (i = 0; i < 3; i++)
      {
      currDist += (hitPosition[i] - origin[i])*(hitPosition[i] - origin[i]);
      }
    currDist = sqrt(currDist)*this->NumberOfDivisions;
    
    // add one offset due to the problems around zero
    for (loop = 0; loop <3; loop++)
      {
      hitPosition[loop] = hitPosition[loop]*this->NumberOfDivisions + 1.0;
      pos[loop] = (int)hitPosition[loop];
      // Adjust right boundary condition: if we intersect from the top, right,
      // or back; then pos must be adjusted to a valid octant index 
      if (pos[loop] > this->NumberOfDivisions)
        {
        pos[loop] = this->NumberOfDivisions;
        }
      }
    
    idx = leafStart + pos[0] - 1 + (pos[1] - 1)*this->NumberOfDivisions 
      + (pos[2] - 1)*prod;
    while ((bestCellId < 0) && (pos[0] > 0) && (pos[1] > 0) && (pos[2] > 0) &&
      (pos[0] <= this->NumberOfDivisions) &&
      (pos[1] <= this->NumberOfDivisions) &&
      (pos[2] <= this->NumberOfDivisions) &&
      (currDist < stopDist))
      {
      if (this->Tree[idx])
        {
        this->ComputeOctantBounds(pos[0]-1, pos[1]-1, pos[2]-1);
        for (tMax = VTK_DOUBLE_MAX, cellId=0; 
        cellId < this->Tree[idx]->GetNumberOfIds(); cellId++) 
          {
          cId = this->Tree[idx]->GetId(cellId);
          if (this->CellHasBeenVisited[cId] != this->QueryNumber)
            {
            this->CellHasBeenVisited[cId] = this->QueryNumber;
            cellIsGhost = 0;
            if (ghosts && this->IgnoreGhosts)
              {
              cellIsGhost = (int)ghosts->GetComponent(cId, 0); 
              }
            if (!cellIsGhost)
              {
              hitCellBounds = 0;
            
              // check whether we intersect the cell bounds
              if (this->CacheCellBounds)
                {
                hitCellBounds = vtkBox::IntersectBox(this->CellBounds[cId],
                                                 a0, direction1,
                                                 hitCellBoundsPosition, result);
                }
              else 
                {
                this->DataSet->GetCellBounds(cId, cellBounds);
                hitCellBounds = vtkBox::IntersectBox(cellBounds,
                                                 a0, direction1,
                                                 hitCellBoundsPosition, result);
                }

              if (hitCellBounds)
                {
                // now, do the expensive GetCell call and the expensive
                // intersect with line call
                this->DataSet->GetCell(cId, cell);
                
                if (IgnoreLines && cell->GetCellType() == VTK_LINE)     
                  continue;

                if (CellIntersectWithLine(cell, a0, a1, tempT, tempX, pc, tempId))
                  {
#if 0
                  if (!this->IsInOctantBounds(tempX))
                   {
                   this->CellHasBeenVisited[cId] = 0; // mark the cell non-visited
                   }
                  else if (tempT < tMax) // it might be close
#endif
                  if (tempT < tMax) // it might be close
                    {
                    tMax = tempT;
                    t = tempT;
                    bestCellId = cId;
                    x[0] = tempX[0]; x[1] = tempX[1]; x[2] = tempX[2];
                    pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = pc[2]; 
                    subId = tempId;
                    } // t <= tMax+tol
                  } // cell Isected line
                } // if (hitCellBounds)
              } // if !cellIsGhost
            } // if (!this->CellHasBeenVisited[cId])
          }
        }
      
      // move to the next octant
      tMax = VTK_DOUBLE_MAX;
      bestDir = 0;
      for (loop = 0; loop < 3; loop++)
        {
        if (direction3[loop] > 0)
          {
          npos[loop] = pos[loop] + 1;
          dist[loop] = (1.0 - hitPosition[loop] + pos[loop])/direction3[loop];
          if (dist[loop] == 0)
            {
            dist[loop] = 1.0/direction3[loop];
            }
          if (dist[loop] < 0)
            {
            dist[loop] = 0;
            }
          if (dist[loop] < tMax)
            {
            bestDir = loop;
            tMax = dist[loop];
            }
          }
        if (direction3[loop] < 0)
          {
          npos[loop] = pos[loop] - 1;
          dist[loop] = (pos[loop] - hitPosition[loop])/direction3[loop];
          if (dist[loop] == 0)
            {
            dist[loop] = -0.01/direction3[loop];
            }
          if (dist[loop] < 0)
            {
            dist[loop] = 0;
            }
          if (dist[loop] < tMax)
            {
            bestDir = loop;
            tMax = dist[loop];
            }
          }
        }
      // update our position
      for (loop = 0; loop < 3; loop++)
        {
        hitPosition[loop] += dist[bestDir]*direction3[loop];
        }
      currDist += dist[bestDir];
      // now make the move, find the smallest distance
      // only cross one boundry at a time
      pos[bestDir] = npos[bestDir];
      
      idx = leafStart + pos[0] - 1 + (pos[1]-1)*this->NumberOfDivisions + 
        (pos[2]-1)*prod;
      }
    }
    
  if (bestCellId >= 0)
    {
    // store the best cell id in the return "parameter"
    cellId = bestCellId;
    }
  cell->Delete();   
  return (bestCellId >= 0);
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

// Return closest point (if any) AND the cell on which this closest point lies
void vtkVisItCellLocator::FindClosestPoint(double x[3], double closestPoint[3], 
                                      vtkGenericCell *cell, vtkIdType &cellId,
                                      int &subId, double& dist2)
{
  int i;
  vtkIdType j;
  int *nei;
  vtkIdType closestCell = -1;
  int closestSubCell = -1;
  int leafStart;
  int level;
  int ijk[3];
  double minDist2, refinedRadius2, distance2ToBucket;
  double distance2ToCellBounds, cellBounds[6];
  double pcoords[3], point[3], cachedPoint[3], weightsArray[6];
  double *weights = weightsArray;
  int nWeights = 6, nPoints, cellIsGhost;
  vtkIdList *cellIds;
  vtkUnsignedCharArray *ghosts =
    (vtkUnsignedCharArray *)this->DataSet->GetCellData()->GetArray("avtGhostZones");
  
  leafStart = this->NumberOfOctants
    - this->NumberOfDivisions*this->NumberOfDivisions*this->NumberOfDivisions;
  
  // Clear the array that indicates whether we have visited this cell.
  // The array is only cleared when the query number rolls over.  This
  // saves a number of calls to memset.
  this->QueryNumber++;
  if (this->QueryNumber == 0)
    {
    this->ClearCellHasBeenVisited();
    this->QueryNumber++;    // can't use 0 as a marker
    }
  
  // init
  dist2 = -1.0;
  refinedRadius2 = VTK_DOUBLE_MAX;
  
  //
  //  Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)((x[j] - this->Bounds[2*j]) / this->H[j]);
    
    if (ijk[j] < 0)
      {
      ijk[j] = 0;
      }
    else if (ijk[j] >= this->NumberOfDivisions)
      {
      ijk[j] = this->NumberOfDivisions-1;
      }
    }
  //
  //  Need to search this bucket for closest point.  If there are no
  //  cells in this bucket, search 1st level neighbors, and so on,
  //  until closest point found.
  //
  for (closestCell=(-1),minDist2=VTK_DOUBLE_MAX,level=0;
  (closestCell == -1) && (level < this->NumberOfDivisions); level++)
    {
    this->GetBucketNeighbors(ijk, this->NumberOfDivisions, level);
    
    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      
      // if a neighboring bucket has cells, 
      if ( (cellIds = 
        this->Tree[leafStart + nei[0] + nei[1]*this->NumberOfDivisions + 
          nei[2]*this->NumberOfDivisions*this->NumberOfDivisions]) != NULL )
        {
        // do we still need to test this bucket?
        distance2ToBucket = this->Distance2ToBucket(x, nei);
        
        if (distance2ToBucket < refinedRadius2)
          {
          // still a viable bucket
          for (j=0; j < cellIds->GetNumberOfIds(); j++) 
            {
            // get the cell
            cellId = cellIds->GetId(j);
            if (this->CellHasBeenVisited[cellId] != this->QueryNumber)
              {
              this->CellHasBeenVisited[cellId] = this->QueryNumber;
              cellIsGhost = 0;
              if (ghosts && this->IgnoreGhosts)
                {
                cellIsGhost = (int) ghosts->GetComponent(cellId, 0);
                }
              
              if (!cellIsGhost)
                {
                // check whether we could be close enough to the cell by
                // testing the cell bounds
                if (this->CacheCellBounds)
                  {
                  distance2ToCellBounds = 
                    this->Distance2ToBounds(x, this->CellBounds[cellId]);
                  }
                else
                  {
                  this->DataSet->GetCellBounds(cellId, cellBounds);
                  distance2ToCellBounds = this->Distance2ToBounds(x, cellBounds);
                  }
              
                if (distance2ToCellBounds < refinedRadius2)
                  {
                  this->DataSet->GetCell(cellId, cell);
                
                  // make sure we have enough storage space for the weights
                  nPoints = cell->GetPointIds()->GetNumberOfIds();
                  if (nPoints > nWeights)
                    {
                    if (nWeights > 6)
                      {
                      delete [] weights;
                      }
                    weights = new double[2*nPoints];  // allocate some extra room
                    nWeights = 2*nPoints;
                    }
                
                  // evaluate the position to find the closest point
                  int stat=cell->EvaluatePosition(x, point, subId, pcoords,
                    dist2, weights);

                  if ( stat != -1 && dist2 < minDist2 ) 
                    {
                    closestCell = cellId;
                    closestSubCell = subId;
                    minDist2 = dist2;
                    cachedPoint[0] = point[0];
                    cachedPoint[1] = point[1];
                    cachedPoint[2] = point[2];
                    refinedRadius2 = dist2;
                    }
                  }
                } // if (!cellIsGhost)
              } // if (!this->CellHasBeenVisited[cellId])
            }
          }
        }
      }
    }
  
  // Because of the relative location of the points in the buckets, the
  // cell found previously may not be the closest cell.  Have to
  // search those bucket neighbors that might also contain nearby cells.
  //
  if ( (minDist2 > 0.0) && (level < this->NumberOfDivisions))
    {
    int prevMinLevel[3], prevMaxLevel[3];
    // setup prevMinLevel and prevMaxLevel to indicate previously visited
    // buckets
    if (--level < 0)
      {
      level = 0;
      }
    for (i = 0; i < 3; i++)
      {
      prevMinLevel[i] = ijk[i] - level;
      if (prevMinLevel[i] < 0)
        {
        prevMinLevel[i] = 0;
        }
      prevMaxLevel[i] = ijk[i] + level;
      if (prevMaxLevel[i] >= this->NumberOfDivisions)
        {
        prevMaxLevel[i] = this->NumberOfDivisions - 1;
        }
      }
    this->GetOverlappingBuckets(x, ijk, sqrt(minDist2), prevMinLevel,
                                prevMaxLevel);
    
    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      
      if ( (cellIds = 
          this->Tree[leafStart + nei[0] + nei[1]*this->NumberOfDivisions + 
            nei[2]*this->NumberOfDivisions*this->NumberOfDivisions]) != NULL )
        {
        // do we still need to test this bucket?
        distance2ToBucket = this->Distance2ToBucket(x, nei);
        
        if (distance2ToBucket < refinedRadius2)
          {
          // still a viable bucket
          for (j=0; j < cellIds->GetNumberOfIds(); j++) 
            {
            // get the cell
            cellId = cellIds->GetId(j);
            if (this->CellHasBeenVisited[cellId] != this->QueryNumber)
              {
              this->CellHasBeenVisited[cellId] = this->QueryNumber;
              
              // check whether we could be close enough to the cell by
              // testing the cell bounds
              if (this->CacheCellBounds)
                {
                distance2ToCellBounds = 
                  this->Distance2ToBounds(x, this->CellBounds[cellId]);
                }
              else
                {
                this->DataSet->GetCellBounds(cellId, cellBounds);
                distance2ToCellBounds = this->Distance2ToBounds(x, cellBounds);
                }
              
              if (distance2ToCellBounds < refinedRadius2)
                {
                this->DataSet->GetCell(cellId, cell);
                
                // make sure we have enough storage space for the weights
                nPoints = cell->GetPointIds()->GetNumberOfIds();
                if (nPoints > nWeights)
                  {
                  if (nWeights > 6)
                    {
                    delete [] weights;
                    }
                  weights = new double[2*nPoints];  // allocate some extra room
                  nWeights = 2*nPoints;
                  }
                
                // evaluate the position to find the closest point
                cell->EvaluatePosition(x, point, subId, pcoords,
                  dist2, weights);
                
                if ( dist2 < minDist2 ) 
                  {
                  closestCell = cellId;
                  closestSubCell = subId;
                  minDist2 = dist2;
                  cachedPoint[0] = point[0];
                  cachedPoint[1] = point[1];
                  cachedPoint[2] = point[2];
                  refinedRadius2 = dist2;
                  }
                }//if point close enough to cell bounds
              }//if cell has not been visited
            }//for each cell
          }//if bucket is still viable
        }//if cells in bucket
      }//for each overlapping bucket
    }//if not identical point
  
  if (closestCell != -1)
    {
    dist2 = minDist2;
    cellId = closestCell;
    subId = closestSubCell;
    closestPoint[0] = cachedPoint[0];
    closestPoint[1] = cachedPoint[1];
    closestPoint[2] = cachedPoint[2];
    this->DataSet->GetCell(cellId, cell);
    }
  
  if (nWeights > 6)
    {
    delete [] weights;
    }
}


// Return closest point (if any) AND the cell on which this closest point lies
void vtkVisItCellLocator::FindClosestPoint(double x[3], double closestPoint[3],
                                      vtkIdType &cellId, int &subId,
                                      double& dist2)
{
  vtkGenericCell *cell = vtkGenericCell::New();
  
  this->FindClosestPoint(x, closestPoint, cell, cellId, subId, dist2);
  
  cell->Delete();
}

int vtkVisItCellLocator::FindClosestPointWithinRadius(double x[3], double radius,
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************
                                                 double closestPoint[3],
                                                 vtkGenericCell *cell,
                                                 vtkIdType &cellId, int &subId,
                                                 double& dist2, int &inside)
{
  int i;
  vtkIdType j;
  int tmpInside;
  int *nei;
  int closestCell = -1;
  int closestSubCell = -1;
  int leafStart;
  int ijk[3];
  double minDist2;
  double pcoords[3], point[3], cachedPoint[3], weightsArray[6];
  double *weights = weightsArray;
  int nWeights = 6, nPoints;
  int returnVal = 0;
  vtkIdList *cellIds; 
  int cellIsGhost;
  vtkUnsignedCharArray *ghosts =
    (vtkUnsignedCharArray *)this->DataSet->GetCellData()->GetArray("avtGhostZones");
  
  double refinedRadius, radius2, refinedRadius2, distance2ToBucket;
  double distance2ToCellBounds, cellBounds[6], currentRadius;
  double distance2ToDataBounds, maxDistance;
  int ii, radiusLevels[3], radiusLevel, prevMinLevel[3], prevMaxLevel[3];
  
  leafStart = this->NumberOfOctants
    - this->NumberOfDivisions*this->NumberOfDivisions*this->NumberOfDivisions;
  
  // Clear the array that indicates whether we have visited this cell.
  // The array is only cleared when the query number rolls over.  This
  // saves a number of calls to memset.
  this->QueryNumber++;
  if (this->QueryNumber == 0)
    {
    this->ClearCellHasBeenVisited();
    this->QueryNumber++;    // can't use 0 as a marker
    }
  
  // init
  dist2 = -1.0;
  closestCell = -1;
  radius2 = radius*radius;
  minDist2 = 1.1*radius2;   // something slightly bigger....
  refinedRadius = radius;
  refinedRadius2 = radius2;
  
  // Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)((x[j] - this->Bounds[2*j]) / this->H[j]);
    
    if (ijk[j] < 0)
      {
      ijk[j] = 0;
      }
    else if (ijk[j] >= this->NumberOfDivisions)
      {
      ijk[j] = this->NumberOfDivisions-1;
      }
    }
  
  // Start by searching the bucket that the point is in.
  //
  if ((cellIds = 
      this->Tree[leafStart + ijk[0] + ijk[1]*this->NumberOfDivisions + 
      ijk[2]*this->NumberOfDivisions*this->NumberOfDivisions]) != NULL )
    {
    // query each cell
    for (j=0; j < cellIds->GetNumberOfIds(); j++) 
      {
      // get the cell
      cellId = cellIds->GetId(j);
      if (this->CellHasBeenVisited[cellId] != this->QueryNumber)
        {
        this->CellHasBeenVisited[cellId] = this->QueryNumber;
        
        // check whether we could be close enough to the cell by
        // testing the cell bounds
        cellIsGhost = 0;
        if (ghosts && this->IgnoreGhosts)
          { 
          cellIsGhost = (int) ghosts->GetComponent(cellId, 0); 
          } 
        if (!cellIsGhost)
          {
          if (this->CacheCellBounds)
            {
            distance2ToCellBounds = 
              this->Distance2ToBounds(x, this->CellBounds[cellId]);
            }
          else
            {
            this->DataSet->GetCellBounds(cellId, cellBounds);
            distance2ToCellBounds = this->Distance2ToBounds(x, cellBounds);
            }
        
          if (distance2ToCellBounds < refinedRadius2)
            {
            this->DataSet->GetCell(cellId, cell);
          
            // make sure we have enough storage space for the weights
            nPoints = cell->GetPointIds()->GetNumberOfIds();
            if (nPoints > nWeights)
              {
              if (nWeights > 6)
                {
                delete [] weights;
                }
              weights = new double[2*nPoints];  // allocate some extra room
              nWeights = 2*nPoints;
              }
          
            // evaluate the position to find the closest point
            tmpInside = cell->EvaluatePosition(x, point, subId, pcoords,
              dist2, weights);
            if ( dist2 < minDist2 ) 
              {
              inside = tmpInside;
              closestCell = cellId;
              closestSubCell = subId;
              minDist2 = dist2;
              cachedPoint[0] = point[0];
              cachedPoint[1] = point[1];
              cachedPoint[2] = point[2];
              refinedRadius = sqrt(dist2);
              refinedRadius2 = dist2;
              }
            }
          } // (!cellIsGhost)
        } // if (this->CellHasBeenVisited[cellId])
      }
    }
  
  // Now, search only those buckets that are within a radius. The radius used
  // is the smaller of sqrt(dist2) and the radius that is passed in. To avoid
  // checking a large number of buckets unnecessarily, if the radius is
  // larger than the dimensions of a bucket, we search outward using a
  // simple heuristic of rings.  This heuristic ends up collecting inner
  // buckets multiple times, but this only happens in the case where these
  // buckets are empty, so they are discarded quickly.
  //
  if (dist2 < radius2 && dist2 >= 0.0)
    {
    refinedRadius = sqrt(dist2);
    refinedRadius2 = dist2;
    }
  else
    {
    refinedRadius = radius;
    refinedRadius2 = radius2;
    }

  
  distance2ToDataBounds = this->Distance2ToBounds(x, this->Bounds);
  maxDistance = sqrt(distance2ToDataBounds) + this->DataSet->GetLength();
  if (refinedRadius > maxDistance)
    {
    refinedRadius = maxDistance;
    refinedRadius2 = maxDistance*maxDistance;
    }
  
  radiusLevels[0] = (int)(refinedRadius/this->H[0]);
  radiusLevels[1] = (int)(refinedRadius/this->H[1]);
  radiusLevels[2] = (int)(refinedRadius/this->H[2]);
  
  radiusLevel = radiusLevels[0];
  radiusLevel = radiusLevels[1] > radiusLevel ? radiusLevels[1] : radiusLevel;
  radiusLevel = radiusLevels[2] > radiusLevel ? radiusLevels[2] : radiusLevel;
  
  if (radiusLevel > this->NumberOfDivisions / 2 )
    {
    radiusLevel = this->NumberOfDivisions / 2;
    }
  if (radiusLevel == 0)
    {
    radiusLevel = 1;
    }

  // radius schedule increases the radius each iteration, this is currently
  // implemented by decreasing ii by 1 each iteration.  another alternative
  // is to double the radius each iteration, i.e. ii = ii >> 1
  // In practice, reducing ii by one has been found to be more efficient.
  int numberOfBucketsPerPlane;
  numberOfBucketsPerPlane = this->NumberOfDivisions*this->NumberOfDivisions;
  prevMinLevel[0] = prevMaxLevel[0] = ijk[0];
  prevMinLevel[1] = prevMaxLevel[1] = ijk[1];
  prevMinLevel[2] = prevMaxLevel[2] = ijk[2];
  for (ii=radiusLevel; ii >= 1; ii--)   
    {
    currentRadius = refinedRadius; // used in if at bottom of this for loop
    
    // Build up a list of buckets that are arranged in rings
    this->GetOverlappingBuckets(x, ijk, refinedRadius/ii, prevMinLevel,
                                prevMaxLevel);
    
    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      
      if ( (cellIds = 
        this->Tree[leafStart + nei[0] + nei[1]*this->NumberOfDivisions + 
          nei[2]*numberOfBucketsPerPlane]) != NULL )
        {
        // do we still need to test this bucket?
        distance2ToBucket = this->Distance2ToBucket(x, nei);
        
        if (distance2ToBucket < refinedRadius2)
          {
          // still a viable bucket
          for (j=0; j < cellIds->GetNumberOfIds(); j++) 
            {
            // get the cell
            cellId = cellIds->GetId(j);
            if (this->CellHasBeenVisited[cellId] != this->QueryNumber)
              {
              this->CellHasBeenVisited[cellId] = this->QueryNumber;
              cellIsGhost = 0;
              if (ghosts && this->IgnoreGhosts)
                { 
                cellIsGhost = (int)ghosts->GetComponent(cellId, 0); 
                } 
              if (!cellIsGhost)
                {
                // check whether we could be close enough to the cell by
                // testing the cell bounds
                if (this->CacheCellBounds)
                  {
                  distance2ToCellBounds = 
                    this->Distance2ToBounds(x, this->CellBounds[cellId]);
                  }
                else
                  {
                  this->DataSet->GetCellBounds(cellId, cellBounds);
                  distance2ToCellBounds = this->Distance2ToBounds(x, cellBounds);
                  }
              
                if (distance2ToCellBounds < refinedRadius2)
                  {
                  this->DataSet->GetCell(cellId, cell);
                
                  // make sure we have enough storage space for the weights
                  nPoints = cell->GetPointIds()->GetNumberOfIds();
                  if (nPoints > nWeights)
                    {
                    if (nWeights > 6)
                      {
                      delete [] weights;
                      }
                    weights = new double[2*nPoints];  // allocate some extra room
                    nWeights = 2*nPoints;
                    }
                
                  // evaluate the position to find the closest point
                  tmpInside = cell->EvaluatePosition(x, point, subId, pcoords,
                    dist2, weights);
                
                  if ( dist2 < minDist2 ) 
                    {
                    inside = tmpInside;
                    closestCell = cellId;
                    closestSubCell = subId;
                    minDist2 = dist2;
                    cachedPoint[0] = point[0];
                    cachedPoint[1] = point[1];
                    cachedPoint[2] = point[2];
                    refinedRadius = sqrt(minDist2);
                    refinedRadius2 = minDist2;
                    }
                  }//if point close enough to cell bounds
                }//if (!cellIsGhost) 
              }//if cell has not been visited
            }//for each cell in bucket
          }//if bucket is within the current best distance
        }//if cells in bucket
      }//for each overlapping bucket

    // don't want to checker a smaller radius than we just checked so update
    // ii appropriately
    if (refinedRadius < currentRadius && ii > 2) //always check ii==1
      {
      ii = (int)((double)ii * (refinedRadius / currentRadius)) + 1;
      if (ii < 2)
        {
        ii = 2;
        }
      }
    }//for each radius in the radius schedule
  
  if ((closestCell != -1) && (minDist2 <= radius2))
    {
    dist2 = minDist2;
    cellId = closestCell;
    subId = closestSubCell;
    closestPoint[0] = cachedPoint[0];
    closestPoint[1] = cachedPoint[1];
    closestPoint[2] = cachedPoint[2];
    this->DataSet->GetCell(cellId, cell);
    returnVal = 1;
    }
  
  if (nWeights > 6)
    {
    delete [] weights;
    }
  
  return returnVal;
}

int vtkVisItCellLocator::FindClosestPointWithinRadius(double x[3], double radius,
                                                 double closestPoint[3],
                                                 vtkGenericCell *cell,
                                                 vtkIdType &cellId,
                                                 int &subId, double& dist2)
{
  int inside;

  return 
    this->FindClosestPointWithinRadius(x, radius, closestPoint,
                                       cell, cellId, subId, dist2, inside);
}

int vtkVisItCellLocator::FindClosestPointWithinRadius(double x[3], double radius,
                                                 double closestPoint[3],
                                                 vtkIdType &cellId, int &subId,
                                                 double& dist2)
{
  vtkGenericCell *cell = vtkGenericCell::New();
  int found, inside;
  
  found = 
    this->FindClosestPointWithinRadius(x, radius, closestPoint,
                                       cell, cellId, subId, dist2, inside);
  cell->Delete();
  return found;
}

//
//  Internal function to get bucket neighbors at specified "level". The
//  bucket neighbors are indices into the "leaf-node" layer of the octree.
//  These indices must be offset by number of octants before the leaf node
//  layer before they can be used. Only those buckets with cells are returned.
//
void vtkVisItCellLocator::GetBucketNeighbors(int ijk[3], int ndivs, int level)
{
  int i, j, k, min, max, minLevel[3], maxLevel[3];
  int nei[3];
  int leafStart;
  int numberOfBucketsPerPlane;
  
  numberOfBucketsPerPlane = this->NumberOfDivisions*this->NumberOfDivisions;
  leafStart = this->NumberOfOctants
    - numberOfBucketsPerPlane*this->NumberOfDivisions;
  
  //  Initialize
  //
  this->Buckets->Reset();

  //  If at this bucket, just place into list
  //
  if ( level == 0 ) 
    {
    if (this->Tree[leafStart + ijk[0] + ijk[1]*this->NumberOfDivisions
      + ijk[2]*numberOfBucketsPerPlane])
      {
      this->Buckets->InsertNextPoint(ijk);
      }
    return;
    }

  //  Create permutations of the ijk indices that are at the level
  //  required. If these are legal buckets, add to list for searching.
  //
  for ( i=0; i<3; i++ ) 
    {
    min = ijk[i] - level;
    max = ijk[i] + level;
    minLevel[i] = ( min > 0 ? min : 0);
    maxLevel[i] = ( max < (ndivs-1) ? max : (ndivs-1));
    }
  
  for ( k= minLevel[2]; k <= maxLevel[2]; k++ ) 
    {
    for ( j= minLevel[1]; j <= maxLevel[1]; j++ ) 
      {
      for ( i= minLevel[0]; i <= maxLevel[0]; i++ ) 
        {
        if (i == (ijk[0] + level) || i == (ijk[0] - level) ||
          j == (ijk[1] + level) || j == (ijk[1] - level) ||
          k == (ijk[2] + level) || k == (ijk[2] - level) ) 
          {
          if (this->Tree[leafStart + i + j*this->NumberOfDivisions
            + k*numberOfBucketsPerPlane])
            {
            nei[0]=i; nei[1]=j; nei[2]=k;
            this->Buckets->InsertNextPoint(nei);
            }
          }
        }
      }
    }
  
  return;
}

// Internal method to find those buckets that are within distance specified.
// Only those buckets outside of level radiuses of ijk are returned. The
// bucket neighbors are indices into the "leaf-node" layer of the octree.
// These indices must be offset by number of octants before the leaf node
// layer before they can be used. Only buckets that have cells are placed
// in the bucket list.
//
void vtkVisItCellLocator::GetOverlappingBuckets(double x[3], int vtkNotUsed(ijk)[3], 
                                           double dist, 
                                           int prevMinLevel[3],
                                           int prevMaxLevel[3])
{
  int i, j, k, nei[3], minLevel[3], maxLevel[3];
  int leafStart, kFactor, jFactor;
  int numberOfBucketsPerPlane, jkSkipFlag, kSkipFlag;

  numberOfBucketsPerPlane = this->NumberOfDivisions*this->NumberOfDivisions;
  leafStart = this->NumberOfOctants
    - numberOfBucketsPerPlane*this->NumberOfDivisions;
  
  // Initialize
  this->Buckets->Reset();
  
  // Determine the range of indices in each direction
  for (i=0; i < 3; i++)
    {
    minLevel[i] = (int) ((double) (((x[i]-dist) - this->Bounds[2*i])
                                  / this->H[i]));
    maxLevel[i] = (int) ((double) (((x[i]+dist) - this->Bounds[2*i])
                                  / this->H[i]));
    
    if ( minLevel[i] < 0 )
      {
      minLevel[i] = 0;
      }
    else if (minLevel[i] >= this->NumberOfDivisions )
      {
      minLevel[i] = this->NumberOfDivisions - 1;
      }
    if ( maxLevel[i] >= this->NumberOfDivisions )
      {
      maxLevel[i] = this->NumberOfDivisions - 1;
      }
    else if ( maxLevel[i] < 0 )
      {
      maxLevel[i] = 0;
      }
    }

  if (minLevel[0] == prevMinLevel[0] && maxLevel[0] == prevMaxLevel[0] &&
      minLevel[1] == prevMinLevel[1] && maxLevel[1] == prevMaxLevel[1] &&
      minLevel[2] == prevMinLevel[2] && maxLevel[2] == prevMaxLevel[2] )
    {
    return;
    }

  for ( k= minLevel[2]; k <= maxLevel[2]; k++ ) 
    {
    kFactor = k*numberOfBucketsPerPlane;
    if (k >= prevMinLevel[2] && k <= prevMaxLevel[2])
      {
      kSkipFlag = 1;
      }
    else
      {
      kSkipFlag = 0;
      }
    for ( j= minLevel[1]; j <= maxLevel[1]; j++ ) 
      {
      if (kSkipFlag && j >= prevMinLevel[1] && j <= prevMaxLevel[1])
        {
        jkSkipFlag = 1;
        }
      else
        {
        jkSkipFlag = 0;
        }
      jFactor = j*this->NumberOfDivisions;
      for ( i= minLevel[0]; i <= maxLevel[0]; i++ ) 
        {
        if ( jkSkipFlag && i == prevMinLevel[0] )
          {
          i = prevMaxLevel[0];
          continue;
          }
        // if this bucket has any cells, add it to the list
        if (this->Tree[leafStart + i + jFactor + kFactor])
          {
          nei[0]=i; nei[1]=j; nei[2]=k;
          this->Buckets->InsertNextPoint(nei);
          }
        }
      }
    }

  prevMinLevel[0] = minLevel[0];
  prevMinLevel[1] = minLevel[1];
  prevMinLevel[2] = minLevel[2];
  prevMaxLevel[0] = maxLevel[0];
  prevMaxLevel[1] = maxLevel[1];
  prevMaxLevel[2] = maxLevel[2];
}

// number of buckets available
int vtkVisItCellLocator::GetNumberOfBuckets(void) 
{
  if (this->Tree)
    {
    return this->NumberOfOctants;
    }
  else
    {
    vtkWarningMacro(<<"Attempting to access Tree before Locator has been built");
    return 0;
    }
}

// Get the cells in a bucket.
vtkIdList* vtkVisItCellLocator::GetCells(int octantId)
{
  // handle parents ?             
  return this->Tree[octantId];
}


//  Method to form subdivision of space based on the cells provided and
//  subject to the constraints of levels and NumberOfCellsPerBucket.
//  The result is directly addressable and of uniform subdivision.
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003
//    Incorporate changes made to vtkCellLocator by Will Schroeder.
//    The changes fix a bug when dataset thickness is very near zero.
//
//    Kathleen Bonnell, Tue Jun  3 15:26:52 PDT 2003 
//    Calculate MinCellLength.
//
//    Kathleen Bonnell, Thu Nov  6 08:29:48 PST 2003 
//    Utilize user-specified-bounds if available. 
//
void vtkVisItCellLocator::BuildLocator()
  {
  double *bounds, length, cellBounds[6], *boundsPtr;
  vtkIdType numCells;
  int ndivs, product;
  int i, j, k, ijkMin[3], ijkMax[3];
  vtkIdType cellId, idx;
  int parentOffset;
  vtkIdList *octant;
  int numCellsPerBucket = this->NumberOfCellsPerBucket;
  typedef vtkIdList *vtkIdListPtr;
  int prod, numOctants;
  double hTol[3];
  
  if ( (this->Tree != NULL) && (this->BuildTime > this->MTime)
    && (this->BuildTime > this->DataSet->GetMTime()) )
    {
    return;
    }
  
  vtkDebugMacro( << "Subdividing octree..." );
  
  if ( !this->DataSet || (numCells = this->DataSet->GetNumberOfCells()) < 1 )
    {
    vtkErrorMacro( << "No cells to subdivide");
    return;
    }

  this->MinCellLength = VTK_DOUBLE_MAX; 
  //  Make sure the appropriate data is available
  //
  if ( this->Tree )
    {
    this->FreeSearchStructure();
    }
  if ( this->CellHasBeenVisited )
    {
    delete [] this->CellHasBeenVisited;
    this->CellHasBeenVisited = NULL;
    }
  if (this->CellBounds)
    {
    delete [] this->CellBounds;
    this->CellBounds = NULL;
    }
  
  //  Size the root cell.  Initialize cell data structure, compute
  //  level and divisions.
  //
  length = this->DataSet->GetLength();
  if (!userBoundsSet)
    {
    bounds = this->DataSet->GetBounds();
    }
  else 
    {
    bounds = this->GetUserBounds();
    double diff, l = 0.0;
    for (i=0; i<3; ++i)
      {
      diff = static_cast<double>(bounds[2*i+1]) -
             static_cast<double>(bounds[2*i]);
      l += diff * diff;
      }
      diff = sqrt(l);
      if (diff > VTK_DOUBLE_MAX)
        l = VTK_DOUBLE_MAX;
      else 
        l= diff;
      //
      // If the user-specified bounds are larger thnt this dataset's bounds,
      // use the dataset bounds.
      //
      if (l > length)
        {
        bounds = this->DataSet->GetBounds(); 
        length = static_cast<double>(l);
        }
    }


  for (i=0; i<3; i++)
    {
    this->Bounds[2*i] = bounds[2*i];
    this->Bounds[2*i+1] = bounds[2*i+1];
    if ((this->Bounds[2*i+1] - this->Bounds[2*i]) <= length/1000.0)
      {
      // bump out the bounds a little of if min==max
      this->Bounds[2*i] -= length/100.0;
      this->Bounds[2*i+1] += length/100.0;
      }
    }
  
  if ( this->Automatic ) 
    {
    this->Level = (int) (ceil(log((double)numCells/numCellsPerBucket) / 
      (log((double) 8.0))));
    } 
  this->Level =(this->Level > this->MaxLevel ? this->MaxLevel : this->Level);
  
  // compute number of octants and number of divisions
  for (ndivs=1,prod=1,numOctants=1,i=0; i<this->Level; i++) 
    {
    ndivs *= 2;
    prod *= 8;
    numOctants += prod;
    }
  this->NumberOfDivisions = ndivs;
  this->NumberOfOctants = numOctants;

  this->Tree = new vtkIdListPtr[numOctants];
  memset (this->Tree, 0, numOctants*sizeof(vtkIdListPtr));
  
  this->CellHasBeenVisited = new unsigned char [ numCells ];
  this->ClearCellHasBeenVisited();
  this->QueryNumber = 0;

  if (this->CacheCellBounds)
    {
    this->CellBounds = new double [numCells][6];
    }
  
  //  Compute width of leaf octant in three directions
  //
  for (i=0; i<3; i++)
    {
    this->H[i] = (this->Bounds[2*i+1] - this->Bounds[2*i]) / ndivs;
    hTol[i] = this->H[i]/100.0;
    }

  //  Insert each cell into the appropriate octant.  Make sure cell
  //  falls within octant.
  //
  parentOffset = numOctants - (ndivs * ndivs * ndivs);
  product = ndivs * ndivs;
  boundsPtr = cellBounds;
  double len2;
  for (cellId=0; cellId<numCells; cellId++) 
    {
    if (this->CacheCellBounds)
      {
      boundsPtr = this->CellBounds[cellId];
      this->DataSet->GetCellBounds(cellId, boundsPtr);
      }
    else
      {
      this->DataSet->GetCellBounds(cellId, cellBounds);
      }

    // Keep track of the minimum cell diagonal length
    //
    len2 = ((cellBounds[1] - cellBounds[0]) * (cellBounds[1] - cellBounds[0])); 
    len2 += ((cellBounds[3] - cellBounds[2]) * (cellBounds[3] - cellBounds[2])); 
    len2 += ((cellBounds[5] - cellBounds[4]) * (cellBounds[5] - cellBounds[4])); 

    if (len2 < this->MinCellLength)
      this->MinCellLength = len2;

    // find min/max locations of bounding box
    for (i=0; i<3; i++)
      {
      ijkMin[i] = (int)((boundsPtr[2*i] - this->Bounds[2*i] - hTol[i]) 
        / this->H[i]);
      ijkMax[i] = (int)((boundsPtr[2*i+1] - this->Bounds[2*i] + hTol[i])
        / this->H[i]);
      
      if (ijkMin[i] < 0)
        {
        ijkMin[i] = 0;
        }
      if (ijkMax[i] >= ndivs)
        {
        ijkMax[i] = ndivs-1;
        }
      }
    
    // each octant inbetween min/max point may have cell in it
    for ( k = ijkMin[2]; k <= ijkMax[2]; k++ )
      {
      for ( j = ijkMin[1]; j <= ijkMax[1]; j++ )
        {
        for ( i = ijkMin[0]; i <= ijkMax[0]; i++ )
          {
          idx = parentOffset + i + j*ndivs + k*product;
          this->MarkParents((void*)VTK_CELL_INSIDE,i,j,k,ndivs,this->Level);
          octant = this->Tree[idx];
          if ( ! octant )
            {
            octant = vtkIdList::New();
            octant->Allocate(numCellsPerBucket,numCellsPerBucket/2);
            this->Tree[idx] = octant;
            }
          octant->InsertNextId(cellId);
          }
        }
      }
    
    } //for all cells
  
  this->BuildTime.Modified();
}

void vtkVisItCellLocator::MarkParents(void* a, int i, int j, int k, 
                                 int ndivs, int level)
{
  int offset, prod, ii;
  vtkIdType parentIdx;
  
  for (offset=0, prod=1, ii=0; ii<level-1; ii++) 
    {
    offset += prod;
    prod = prod << 3;
    }
  
  while ( level > 0  )
    {
    i = i >> 1;
    j = j >> 1;
    k = k >> 1;
    ndivs = ndivs >> 1;
    level--;
    
    parentIdx = offset + i + j*ndivs + k*ndivs*ndivs;
    
    // if it already matches just return
    if (a == this->Tree[parentIdx])
      {
      return;
      }
    
    this->Tree[parentIdx] = (vtkIdList *)a;
    
    prod = prod >> 3;
    offset -= prod;
    }
}

void vtkVisItCellLocator::GenerateRepresentation(int level, vtkPolyData *pd)
{
  vtkPoints *pts;
  vtkCellArray *polys;
  int l, i, j, k, ii, boundary[3];
  vtkIdType idx;
  vtkIdList *inside, *Inside[3];
  int numDivs=1;
  
  if ( this->Tree == NULL )
    {
    vtkErrorMacro(<<"No tree to generate representation from");
    return;
    }
  
  pts = vtkPoints::New();
  pts->Allocate(5000);
  polys = vtkCellArray::New();
  polys->Allocate(10000);
  
  // Compute idx into tree at appropriate level; determine if
  // faces of octants are visible.
  //
  int parentIdx = 0;
  int numOctants = 1;
  
  if ( level < 0 )
    {
    level = this->Level;
    }
  for (l=0; l < level; l++)
    {
    numDivs *= 2;
    parentIdx += numOctants;
    numOctants *= 8;
    }
  
  //loop over all octabts generating visible faces
  for ( k=0; k < numDivs; k++)
    {
    for ( j=0; j < numDivs; j++)
      {
      for ( i=0; i < numDivs; i++)
        {
        this->GenerateIndex(parentIdx,numDivs,i,j,k,idx);
        inside = this->Tree[idx];
        
        if ( !(boundary[0]=this->GenerateIndex(parentIdx,numDivs,i-1,j,k,idx)))
          {
          Inside[0] = this->Tree[idx];
          }
        if ( !(boundary[1]=this->GenerateIndex(parentIdx,numDivs,i,j-1,k,idx)))
          {
          Inside[1] = this->Tree[idx];
          }
        if ( !(boundary[2]=this->GenerateIndex(parentIdx,numDivs,i,j,k-1,idx)))
          {
          Inside[2] = this->Tree[idx];
          }
        
        for (ii=0; ii < 3; ii++)
          {
          if ( boundary[ii] )
            {
            if ( inside )
              {
              this->GenerateFace(ii,numDivs,i,j,k,pts,polys);
              }
            }
          else
            {
            if ( (Inside[ii] && !inside) || (!Inside[ii] && inside) )
              {
              this->GenerateFace(ii,numDivs,i,j,k,pts,polys);
              }
            }
          //those buckets on "positive" boundaries can generate faces specially
          if ( (i+1) >= numDivs && inside )
            {
            this->GenerateFace(0,numDivs,i+1,j,k,pts,polys);
            }
          if ( (j+1) >= numDivs && inside )
            {
            this->GenerateFace(1,numDivs,i,j+1,k,pts,polys);
            }
          if ( (k+1) >= numDivs && inside )
            {
            this->GenerateFace(2,numDivs,i,j,k+1,pts,polys);
            }
          
          }//over negative faces
        }//over i divisions
      }//over j divisions
    }//over k divisions
  
  pd->SetPoints(pts);
  pts->Delete();
  pd->SetPolys(polys);
  polys->Delete();
  pd->Squeeze();
}
  
void vtkVisItCellLocator::GenerateFace(int face, int numDivs, int i, int j, int k,
                                  vtkPoints *pts, vtkCellArray *polys)
{
  int ii;
  vtkIdType ids[4];
  double origin[3], x[3];
  double h[3];

  // define first corner; use ids[] as temporary array
  ids[0] = i; ids[1] = j; ids[2] = k;
  for (ii=0; ii<3; ii++)
    {
    h[ii] = (this->Bounds[2*ii+1] - this->Bounds[2*ii]) / numDivs;
    origin[ii] = this->Bounds[2*ii] + ids[ii]*h[ii];
    }

  ids[0] = pts->InsertNextPoint(origin);

  if ( face == 0 ) //x face
    {
    x[0] = origin[0];
    x[1] = origin[1] + h[1];
    x[2] = origin[2];
    ids[1] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1] + h[1];
    x[2] = origin[2] + h[2];
    ids[2] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1];
    x[2] = origin[2] + h[2];
    ids[3] = pts->InsertNextPoint(x);
    }

  else if ( face == 1 ) //y face
    {
    x[0] = origin[0] + h[0];
    x[1] = origin[1];
    x[2] = origin[2];
    ids[1] = pts->InsertNextPoint(x);

    x[0] = origin[0] + h[0];
    x[1] = origin[1];
    x[2] = origin[2] + h[2];
    ids[2] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1];
    x[2] = origin[2] + h[2];
    ids[3] = pts->InsertNextPoint(x);
    }

  else //z face
    {
    x[0] = origin[0] + h[0];
    x[1] = origin[1];
    x[2] = origin[2];
    ids[1] = pts->InsertNextPoint(x);

    x[0] = origin[0] + h[0];
    x[1] = origin[1] + h[1];
    x[2] = origin[2];
    ids[2] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1] + h[1];
    x[2] = origin[2];
    ids[3] = pts->InsertNextPoint(x);
    }

  polys->InsertNextCell(4,ids);
}
  
void vtkVisItCellLocator::ClearCellHasBeenVisited()
{
  if (this->CellHasBeenVisited && this->DataSet)
    {
    memset(this->CellHasBeenVisited, 0, this->DataSet->GetNumberOfCells());
    }
}
  
void vtkVisItCellLocator::ClearCellHasBeenVisited(int id)
{
  if (this->CellHasBeenVisited
      && this->DataSet && id < this->DataSet->GetNumberOfCells())
    {
    this->CellHasBeenVisited[id] = 0;
    }
}
  
// Calculate the distance between the point x to the bucket "nei".
//
// WARNING!!!!! Be very careful altering this routine.  Simple changes to this
// routine can make is 25% slower!!!!
//
double vtkVisItCellLocator::Distance2ToBucket(double x[3], int nei[3])
{
  double bounds[6];

  bounds[0] =     nei[0]*this->H[0] + this->Bounds[0];
  bounds[1] = (nei[0]+1)*this->H[0] + this->Bounds[0];
  bounds[2] =     nei[1]*this->H[1] + this->Bounds[2];
  bounds[3] = (nei[1]+1)*this->H[1] + this->Bounds[2];
  bounds[4] =     nei[2]*this->H[2] + this->Bounds[4];
  bounds[5] = (nei[2]+1)*this->H[2] + this->Bounds[4];

  return this->Distance2ToBounds(x, bounds);
}

// Calculate the distance between the point x and the specified bounds
//
// WARNING!!!!! Be very careful altering this routine.  Simple changes to this
// routine can make it 25% slower!!!!
double vtkVisItCellLocator::Distance2ToBounds(double x[3], double bounds[6])
{
  double distance;
  double deltas[3];

  // Are we within the bounds?
  if (x[0] >= bounds[0] && x[0] <= bounds[1]
    && x[1] >= bounds[2] && x[1] <= bounds[3]
    && x[2] >= bounds[4] && x[2] <= bounds[5])
    {
    return 0.0;
    }

  deltas[0] = deltas[1] = deltas[2] = 0.0;

  // dx
  //
  if (x[0] < bounds[0])
    {
    deltas[0] = bounds[0] - x[0];
    }
  else if (x[0] > bounds[1])
    {
    deltas[0] = x[0] - bounds[1];
    }

  // dy
  //
  if (x[1] < bounds[2])
    {
    deltas[1] = bounds[2] - x[1];
    }
  else if (x[1] > bounds[3])
    {
    deltas[1] = x[1] - bounds[3];
    }

  // dz
  //
  if (x[2] < bounds[4])
    {
    deltas[2] = bounds[4] - x[2];
    }
  else if (x[2] > bounds[5])
    {
    deltas[2] = x[2] - bounds[5];
    }

  distance = vtkMath::Dot(deltas, deltas);
  return distance;
}


void vtkVisItCellLocator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Number of Cells Per Bucket: " 
     << this->NumberOfCellsPerBucket << "\n";
  os << indent << "Cache Cell Bounds: " << this->CacheCellBounds << "\n";  
}
 



static
double
GetParametricDistance(vtkCell *cell, double pcoords[3])
{
  int i;
  double pDist, pDistMax = 0.0f;
  double pc[4];
  switch (cell->GetCellType())
    {
    case VTK_TRIANGLE :
    case VTK_QUADRATIC_TRIANGLE :
      {
      pc[0] = pcoords[0];    
      pc[1] = pcoords[1];    
      pc[2] = 1.0 - pcoords[0] - pcoords[1];    

      for (i=0; i<3; i++)
        {
        if ( pc[i] < 0.0 ) 
          {
          pDist = -pc[i];
          }
        else if ( pc[i] > 1.0 ) 
          {
          pDist = pc[i] - 1.0f;
          }
        else //inside the cell in the parametric direction
          {
          pDist = 0.0;
          }
        if ( pDist > pDistMax )
          {
          pDistMax = pDist;
          }
        }
      }
      break;
    case VTK_TETRA :
    case VTK_QUADRATIC_TETRA :
      {
      pc[0] = pcoords[0];    
      pc[1] = pcoords[1];    
      pc[2] = pcoords[2];    
      pc[3] = 1.0 - pcoords[0] - pcoords[1] - pcoords[2];    
      for (i=0; i<4; i++)
        {
        if ( pc[i] < 0.0 ) 
          {
          pDist = -pc[i];
          }
        else if ( pc[i] > 1.0 ) 
          {
          pDist = pc[i] - 1.0f;
          }
        else //inside the cell in the parametric direction
          {
          pDist = 0.0;
          }
        if ( pDist > pDistMax )
          {
          pDistMax = pDist;
          }
        }
      }
      break;
    default :
      {
      pc[0] = pcoords[0];    
      pc[1] = pcoords[1];    
      pc[2] = pcoords[2];    
      for (i=0; i<3; i++)
        {
        if ( pc[i] < 0.0 ) 
          {
          pDist = -pc[i];
          }
        else if ( pc[i] > 1.0 ) 
          {
          pDist = pc[i] - 1.0f;
          }
        else //inside the cell in the parametric direction
          {
          pDist = 0.0;
          }
        if ( pDist > pDistMax )
          {
          pDistMax = pDist;
          }
        }
      }
      break;
    }
  return pDistMax;
}

void vtkVisItCellLocator::ComputeOctantBounds(int i, int j, int k)
{
  this->OctantBounds[0] = this->Bounds[0]       + i*H[0];
  this->OctantBounds[1] = this->OctantBounds[0] + H[0];
  this->OctantBounds[2] = this->Bounds[2]       + j*H[1];
  this->OctantBounds[3] = this->OctantBounds[2] + H[1];
  this->OctantBounds[4] = this->Bounds[4]       + k*H[2];
  this->OctantBounds[5] = this->OctantBounds[4] + H[2];
}

int
vtkVisItCellLocator::CellIntersectWithLine(vtkCell *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pc[3], int& sub)
{
  switch(cell->GetCellType())
    {
    case VTK_EMPTY_CELL : return 0;
    case VTK_VERTEX : 
      return VertexIntersectWithLine((vtkVertex*)cell, p1, p2, t, x, pc, sub);
    case VTK_POLY_VERTEX : 
      return PolyVertexIntersectWithLine((vtkPolyVertex*)cell, p1, p2, t, x, pc, sub);
    case VTK_LINE : 
      return LineIntersectWithLine((vtkLine*)cell, p1, p2, t, x, pc, sub);
    case VTK_POLY_LINE : 
      return PolyLineIntersectWithLine((vtkPolyLine*)cell, p1, p2, t, x, pc, sub);
    case VTK_TRIANGLE : 
      return TriangleIntersectWithLine((vtkTriangle*)cell, p1, p2, t, x, pc, sub);
    case VTK_TRIANGLE_STRIP : 
      return TriStripIntersectWithLine((vtkTriangleStrip*)cell, p1, p2, t, x, pc, sub);
    case VTK_POLYGON : 
      return PolygonIntersectWithLine((vtkPolygon*)cell, p1, p2, t, x, pc, sub);
    case VTK_PIXEL : 
      return PixelIntersectWithLine((vtkPixel*)cell, p1, p2, t, x, pc, sub);
    case VTK_QUAD : 
      return QuadIntersectWithLine((vtkQuad*)cell, p1, p2, t, x, pc, sub);
    case VTK_TETRA : 
      return TetraIntersectWithLine((vtkTetra*)cell, p1, p2, t, x, pc, sub);
    case VTK_VOXEL : 
      return VoxelIntersectWithLine((vtkVoxel*)cell, p1, p2, t, x, pc, sub);
    case VTK_HEXAHEDRON : 
      return HexIntersectWithLine((vtkHexahedron*)cell, p1, p2, t, x, pc, sub);
    case VTK_WEDGE : 
      return WedgeIntersectWithLine((vtkWedge*)cell, p1, p2, t, x, pc, sub);
    case VTK_PYRAMID : 
      return PyramidIntersectWithLine((vtkPyramid*)cell, p1, p2, t, x, pc, sub);
    default:
      vtkDebugMacro( << "CellType  " << cell->GetCellType() 
                     << "not yet supported for CellIntersectWithLine ..." );
      return 0;
    }
}

int
vtkVisItCellLocator::VertexIntersectWithLine(vtkVertex *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  double cp[3];
  double X[3];
  cell->Points->GetPoint(0, X);
  double dist = vtkLine::DistanceToLine(X, p1, p2, t, cp);
  pcoords[1] = pcoords[2] = 0.;
  if (dist == 0)
    {
    x[0] = X[0];
    x[1] = X[1];
    x[2] = X[2];
    pcoords[0] =  0.;
    return 1;
    }
  pcoords[0] =  -10.;
  return 0;
}

int
vtkVisItCellLocator::PolyVertexIntersectWithLine(vtkPolyVertex *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int subTest, numPts=cell->Points->GetNumberOfPoints();
  
  vtkVertex *vertex = vtkVertex::New();
  double pt[3];
  for (subId=0; subId < numPts; subId++)
    {
    cell->Points->GetPoint(subId, pt);
    vertex->Points->SetPoint(0, pt);

    if ( VertexIntersectWithLine(vertex, p1, p2, t, x, pcoords, subTest) )
      {
      return 1;
      }
    }

  return 0;
}

int
vtkVisItCellLocator::LineIntersectWithLine(vtkLine *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  double a1[3], a2[3];
  int i;

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  cell->Points->GetPoint(0, a1);
  cell->Points->GetPoint(1, a2);

  if ( vtkLine::Intersection(p1, p2, a1, a2, t, pcoords[0]) == 2 )
    {
    for (i=0; i<3; i++)
      {
      x[i] = a1[i] + pcoords[0]*(a2[i]-a1[i]);
      }
    return 1;
    }
  return 0;
}

int
vtkVisItCellLocator::PolyLineIntersectWithLine(vtkPolyLine *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int i, numLines=cell->Points->GetNumberOfPoints() - 1;
  double a1[3], a2[3];
  for (subId=0; subId < numLines; subId++)
    {
    cell->Points->GetPoint(subId, a1);
    cell->Points->GetPoint(subId+1, a2);
    if ( vtkLine::Intersection(p1, p2, a1, a2, t, pcoords[0]) == 2)
      {
      for (i=0; i<3; i++)
        {
        x[i] = a1[i] + pcoords[0]*(a2[i]-a1[i]);
        }
      return 1;
      }
    }
 
  return 0;
}

#define SUB(dest,v1,v2){dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2];}

// ****************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Mon Jul  7 15:57:37 PDT 2003 
//   Use smaller eps for testing when DotProduct close to zero.
//
//   Kathleen Bonnell, Thu Sep 18 15:48:54 PDT 2003
//   Cast multiplication to double before setting intersection point. 
//
//   Kathleen Bonnell, Fri Oct 10 10:46:48 PDT 2003 
//   Remove eps for testing DotProduct.
//
//   Kathleen Bonnell, Tue Jul 27 11:06:24 PDT 2004 
//   If the line is coplanar with the triangle, and we should test for
//   intersection in this case, call EdgeLineIsect. 
//
// ****************************************************************************
int
vtkVisItCellLocator::TriangleIntersectWithLine(vtkTriangle *cell, double p1[3], 
    double p2[3], double& t, double x[3], double pcoords[3], int& subId)
{
  double pt1[3], pt2[3], pt3[3];
  cell->Points->GetPoint(0, pt1);
  cell->Points->GetPoint(1, pt2);
  cell->Points->GetPoint(2, pt3);
 
  double e1[3], e2[3], p[3], s[3], q[3];
  double rayDir[3];
  double u, v, tmp, dt;

  int i;
  SUB(rayDir, p2, p1);
  SUB(e1, pt2, pt1);
  SUB(e2, pt3, pt1);

  vtkMath::Cross(rayDir, e2, p);
  tmp = vtkMath::Dot(p, e1);

  if (tmp == 0.)
  {
      if (this->TestCoPlanar)
          return EdgeLineIsect(cell, p1, p2, x);
      else
          return 0;
  }

  tmp = 1.0/tmp;
  SUB(s, p1, pt1);

  u = tmp * vtkMath::Dot(s, p);

  if (u < 0.0 || u > 1.0)
    {
    pcoords[0] = pcoords[1] = pcoords[2] = 0.0;
    return 0;
    }
 
  vtkMath::Cross(s, e1, q);
  v = tmp * vtkMath::Dot(rayDir, q);

  if (v < 0. || v > 1.) 
    {
    pcoords[0] = pcoords[1] = pcoords[2] = 0.0;
    return 0;
    }
  if ((u+v) > 1.)
    {
    pcoords[0] = pcoords[1] = pcoords[2] = 0.0;
    return 0;
    }
  dt = tmp * vtkMath::Dot(e2, q);

  pcoords[0] = u;
  pcoords[1] = v;
  pcoords[2] = 0; 

  for (i = 0; i < 3; i++)
    {
    x[i] = (double) (p1[i] + (double)(dt*rayDir[i]));
    }
  t = (double)dt;

  return 1;
}

int
vtkVisItCellLocator::TriStripIntersectWithLine(vtkTriangleStrip *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int subTest, numTris = cell->Points->GetNumberOfPoints()-2;
  double a1[3], a2[3], a3[3];
  for (subId = 0; subId < numTris; subId++)
    {
    cell->Points->GetPoint(subId, a1);
    cell->Points->GetPoint(subId+1, a2);
    cell->Points->GetPoint(subId+2, a3);
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
 
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, t, x, pcoords, subTest) )
      {
      return 1;
      }
    }
 
  return 0;
}

// ****************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 27 11:06:24 PDT 2004 
//   If the line is coplanar with the polygon, and we should test for
//   intersection in this case, call EdgeLineIsect. 
//
// ****************************************************************************

int
vtkVisItCellLocator::PolygonIntersectWithLine(vtkPolygon *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  //
  // Only change from Polygon::IntersectWithLine is dependence upon tol.
  //
  double pt1[3], n[3];
  double closestPoint[3];
  double dist2;
  int npts = cell->GetNumberOfPoints();
  double *weights;
 
  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = 0.0;
 
  // Define a plane to intersect with
  //
  cell->Points->GetPoint(1, pt1);
  cell->ComputeNormal(cell->Points,n);
  double tmpt = -1; 
  // Intersect plane of the polygon with line
  //
  if ( ! vtkPlane::IntersectWithLine(p1,p2,n,pt1,tmpt,x) )
    {
      if (tmpt != VTK_DOUBLE_MAX || !this->TestCoPlanar)
          return 0;

       return EdgeLineIsect(cell, p1, p2, x);
    }

  t = tmpt; 
  // Evaluate position
  //
  weights = new double[npts];
  if ( cell->EvaluatePosition(x, closestPoint, subId, pcoords, dist2, weights))
    {
      delete [] weights;
      return 1;
    }
  delete [] weights;
  return 0;
}

// ****************************************************************************
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 27 11:06:24 PDT 2004 
//   If the line is coplanar with the pixel, and we should test for
//   intersection in this case, call EdgeLineIsect. 
//
// ****************************************************************************
int
vtkVisItCellLocator::PixelIntersectWithLine(vtkPixel *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  // rewriting pixel code only so that tol is not needed.
  double pt1[3], pt4[3], n[3];
  double closestPoint[3];
  double dist2, weights[4];
  int i;
  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = 0.0;

  // 
  // Get normal for triangle
  // 
  cell->Points->GetPoint(0, pt1);
  cell->Points->GetPoint(3, pt4);
 
  n[0] = n[1] = n[2] = 0.0;
  for (i=0; i<3; i++)
    {
    if ( (pt4[i] - pt1[i]) <= 0.0 )
      {
      n[i] = 1.0;
      break;
      }
    }
  //
  // Intersect plane of pixel with line.
  //
  double tmpt = -1;
  if ( ! vtkPlane::IntersectWithLine(p1,p2,n,pt1,tmpt,x) )
  {
      if (tmpt != VTK_DOUBLE_MAX || !this->TestCoPlanar)
          return 0;
      return EdgeLineIsect(cell, p1, p2, x);
    }
  t = tmpt;
  //
  // Does intersection point lie within pixel? 
  //
  if (cell->EvaluatePosition(x, closestPoint, subId, pcoords, dist2, weights) )
    {
    return 1;
    }
  return 0;

}

int
vtkVisItCellLocator::QuadIntersectWithLine(vtkQuad *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  double a1[3], a2[3], a3[3], a4[3];
  int diagonalCase;
  cell->Points->GetPoint(0, a1);
  cell->Points->GetPoint(1, a2);
  cell->Points->GetPoint(2, a3);
  cell->Points->GetPoint(3, a4);
  double d1 = vtkMath::Distance2BetweenPoints(a1, a3);
  double d2 = vtkMath::Distance2BetweenPoints(a2, a4);
  subId = 0;

  // Figure out how to uniquely tessellate the quad. Watch out for 
  // equivalent triangulations (i.e., the triangulation is equivalent
  // no matter where the diagonal). In this case use the point ids as 
  // a tie breaker to insure unique triangulation across the quad.
  //
  if ( d1 == d2 ) //rare case; discriminate based on point id
    {
    int i, id, maxId=0, maxIdx=0;
    for (i=0; i<4; i++) //find the maximum id
      {
      if ( (id=cell->PointIds->GetId(i)) > maxId )
        {
        maxId = id;
        maxIdx = i;
        }
      }
    if ( maxIdx == 0 || maxIdx == 2) diagonalCase = 0;
    else diagonalCase = 1;
    }
  else if ( d1 < d2 )
    {
    diagonalCase = 0;
    }
  else //d2 < d1
    {
    diagonalCase = 1;
    }

  // Note: in the following code the parametric coords must be adjusted to
  // reflect the use of the triangle parametric coordinate system.
  switch (diagonalCase)
    {
    case 0:
      this->triangle->Points->SetPoint(0,a1);
      this->triangle->Points->SetPoint(1,a2);
      this->triangle->Points->SetPoint(2,a3);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x, pcoords, subId) )
        {
        pcoords[0] = pcoords[0] + pcoords[1];
        return 1;
        }
      this->triangle->Points->SetPoint(0,a3);
      this->triangle->Points->SetPoint(1,a4);
      this->triangle->Points->SetPoint(2,a1);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x, pcoords, subId)   )
        {
        pcoords[0] = 1.0 - (pcoords[0]+pcoords[1]);
        pcoords[1] = 1.0 - pcoords[1];
        return 1;
        }
      return 0;

    case 1:
      this->triangle->Points->SetPoint(0,a1);
      this->triangle->Points->SetPoint(1,a2);
      this->triangle->Points->SetPoint(2,a4);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x, pcoords, subId) )
        {  
        return 1;
        }
      this->triangle->Points->SetPoint(0,a3);
      this->triangle->Points->SetPoint(1,a4);
      this->triangle->Points->SetPoint(2,a2);
      if (TriangleIntersectWithLine(this->triangle, p1, p2, t, x, pcoords, subId) )
        {
        pcoords[0] = 1.0 - pcoords[0];
        pcoords[1] = 1.0 - pcoords[1];
        return 1;
        }
      return 0;
    }  
    return 0;
}

int
vtkVisItCellLocator::TetraIntersectWithLine(vtkTetra *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int intersection = 0;
  double tTemp, pc[3], xTemp[3], a1[3], a2[3], a3[3];
  int faceNum, *faceIds;

  t = VTK_DOUBLE_MAX;

  for (faceNum = 0; faceNum < 4; faceNum++)
    {
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp, pc, subId))
      {
      intersection = 1;
      if (tTemp < t)
        {
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        switch (faceNum)
          {
          case 0:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 0.0;
            break;
 
          case 1:
            pcoords[0] = 0.0; pcoords[1] = pc[1]; pcoords[2] = 0.0;
            break;
 
          case 2:
            pcoords[0] = pc[0]; pcoords[1] = 0.0; pcoords[2] = 0.0;
            break;
 
          case 3:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = pc[2];
            break;
          }
        }
      }
    }
  return intersection;
}

int
vtkVisItCellLocator::VoxelIntersectWithLine(vtkVoxel *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  double minPt[3], maxPt[3];
  double bounds[6], p21[3];
  int i;
 
  subId = 0;
 
  cell->Points->GetPoint(0, minPt);
  cell->Points->GetPoint(7, maxPt);
 
  for (i=0; i<3; i++)
    {
    p21[i] = p2[i] - p1[i];
    bounds[2*i] = minPt[i];
    bounds[2*i+1] = maxPt[i];
    }
 
  if ( ! vtkBox::IntersectBox(bounds, p1, p21, x, t) )
    {
    return 0;
    }
  //
  // Evaluate intersection
  //
  for (i=0; i<3; i++)
    {
    pcoords[i] = (x[i] - minPt[i]) / (maxPt[i] - minPt[i]);
    }
 
  return 1;
}

int
vtkVisItCellLocator::HexIntersectWithLine(vtkHexahedron *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int intersection = 0, faceNum, *faceIds;
  double tTemp, xTemp[3], pc[3], a1[3], a2[3], a3[3], a4[4];
  t = VTK_DOUBLE_MAX;
  for (faceNum = 0; faceNum < 6; faceNum++)
    {
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    cell->Points->GetPoint(faceIds[3], a4);
    this->quad->Points->SetPoint(0, a1);
    this->quad->Points->SetPoint(1, a2);
    this->quad->Points->SetPoint(2, a3);
    this->quad->Points->SetPoint(3, a4);
    if (this->QuadIntersectWithLine(quad, p1, p2, tTemp, xTemp, pc, subId))
      {
      intersection = 1;
      if (tTemp < t)
        {
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        switch(faceNum)
          {
          case 0:
            pcoords[0] = 0.0; pcoords[0] = pc[0]; pcoords[1] = 0.0;
            break;
 
          case 1:
            pcoords[0] = 1.0; pcoords[0] = pc[0]; pcoords[1] = 0.0;
            break;
 
          case 2:
            pcoords[0] = pc[0]; pcoords[1] = 0.0; pcoords[2] = pc[1];
            break;
 
          case 3:
            pcoords[0] = pc[0]; pcoords[1] = 1.0; pcoords[2] = pc[1];
            break;
 
          case 4:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 0.0;
            break;
 
          case 5:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 1.0;
          }
        }
      }
    }
  return intersection;  
}

int
vtkVisItCellLocator::WedgeIntersectWithLine(vtkWedge *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int intersection = 0;
  double tTemp, xTemp[3], pc[3], a1[3], a2[3], a3[3], a4[3];
  int faceNum, *faceIds;

  t = VTK_DOUBLE_MAX;

  // faces 0 & 1 are triangle-faces, intersect them first
  for (faceNum = 0; faceNum < 2; faceNum++)
    {
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1); 
    cell->Points->GetPoint(faceIds[1], a2); 
    cell->Points->GetPoint(faceIds[2], a3); 
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp, pc, subId))
      {
      intersection = 1;
      if (tTemp < t)
        {
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        switch (faceNum)
          {
          case 0:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 0.0;
            break;
 
          case 1:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 1.0;
            break;
          }
        }
      }
    }

  // faces 2 -4 are quads, now intersect them
  for (faceNum = 2; faceNum < 5; faceNum++)
    {
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    cell->Points->GetPoint(faceIds[3], a4);
    this->quad->Points->SetPoint(0, a1);
    this->quad->Points->SetPoint(1, a2);
    this->quad->Points->SetPoint(2, a3);
    this->quad->Points->SetPoint(3, a4);
    if (this->QuadIntersectWithLine(this->quad, p1, p2, tTemp, xTemp, pcoords, subId))
      {
      intersection = 1;
      if (tTemp < t)
        {
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        switch (faceNum)
          {
          case 2:
            pcoords[0] = pc[1]; pcoords[1] = 0.0; pcoords[2] = pc[0];
            break;
 
          case 3:
            pcoords[0] = 1.0-pc[1]; pcoords[1] = pc[1]; pcoords[2] = pc[0];
            break;
 
          case 4:
            pcoords[0] = 0.0; pcoords[1] = pc[1]; pcoords[2] = pc[0];
            break;
          }
        }
      }
    }
  return intersection;
}

int
vtkVisItCellLocator::PyramidIntersectWithLine(vtkPyramid *cell, double p1[3], double p2[3], 
    double& t, double x[3], double pcoords[3], int& subId)
{
  int intersection = 0;
  int faceNum;
  int *faceIds;
  double xTemp[3], weights[5], pc[3], dist2, tTemp;
  double a1[3], a2[3], a3[3], a4[3];

  t = VTK_DOUBLE_MAX;

  // faces 1-4 are triangles, intersect them first.
  for (faceNum = 1; faceNum < 5; faceNum++)
    {
    faceIds = cell->GetFaceArray(faceNum);
    cell->Points->GetPoint(faceIds[0], a1);
    cell->Points->GetPoint(faceIds[1], a2);
    cell->Points->GetPoint(faceIds[2], a3);
    this->triangle->Points->SetPoint(0, a1);
    this->triangle->Points->SetPoint(1, a2);
    this->triangle->Points->SetPoint(2, a3);
    if (this->TriangleIntersectWithLine(this->triangle, p1, p2, tTemp, xTemp, pc, subId))
      {
      intersection = 1;
      if (tTemp < t)
        {
        t = tTemp;
        x[0] = xTemp[0];
        x[1] = xTemp[1];
        x[2] = xTemp[2];
        //
        // to get the correct pcoords
        //
        cell->EvaluatePosition(x, xTemp, subId, pcoords, dist2, weights);
        }
      }
    }
  // now intersect quad face 
  faceIds = cell->GetFaceArray(0);
  cell->Points->GetPoint(faceIds[0], a1);
  cell->Points->GetPoint(faceIds[1], a2);
  cell->Points->GetPoint(faceIds[2], a3);
  cell->Points->GetPoint(faceIds[3], a4);
  this->quad->Points->SetPoint(0, a1);
  this->quad->Points->SetPoint(1, a2);
  this->quad->Points->SetPoint(2, a3);
  this->quad->Points->SetPoint(3, a4);

  if (this->QuadIntersectWithLine(this->quad, p1, p2, tTemp, xTemp, pc, subId))
    {
    intersection = 1;
    if (tTemp < t)
      {
      t = tTemp;
      x[0] = xTemp[0];
      x[1] = xTemp[1];
      x[2] = xTemp[2];
      pcoords[0] = pc[0];
      pcoords[1] = pc[1];
      pcoords[2] = 0.0; 
      }
    }

  return intersection; 
}


// ****************************************************************************
// Purpose:
//   Allows user to set bounds that locator will use in building the search
//   structure.  Useful if the dataset contains a lot of points not associated
//   with cells.
// 
// Creation:  Kathleen Bonnell, November 4, 2003
//
// ****************************************************************************

void 
vtkVisItCellLocator::SetUserBounds(double b[6])
{
    for (int i = 0; i < 6; ++i)
        this->UserBounds[i] = b[i];
    
    userBoundsSet = true;
}


// ****************************************************************************
// Method:    LineLineIsect
//
// Description:
//   Determines if two line segments intersect.
//
//   From Graphics Gems II, "Intersection of Line Segments" by Mukesh Prasad, 
//   p. 7-9, code: p. 473-476, xlines.c.
//
// Returns:     1 if an intersection is found, 0 otherwise.
//
// Arguments:
//   p1         The first endpoint of the first line segment. 
//   p2         The second endpoint of the first line segment.
//   p3         The first endpoint of the second line segment. 
//   p4         The second endpoint of the second line segment.
//   isect      A place to store the intersection point, if any. 
//
// Programmer:  Kathleen Bonnell
// Creation:    July 27, 2004 
//
// ****************************************************************************

int
LineLineIsect(const double *p1, const double *p2, const double *p3, 
              const double *p4, double *isect)
{
    double a1, a2, b1, b2, c1, c2, r1, r2, r3, r4;
    double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0]; 
    double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1]; 

    a1 = y2 - y1; 
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    r3 = a1 * x3 + b1 *y3 + c1;
    r4 = a1 * x4 + b1 *y4 + c1;
           
    if ((r3 < 0 && r4 < 0) || (r3 > 0 && r4 > 0))
    {
        return 0;
    }

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    r1 = a2 * x1 + b2 * y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;
            
    if ((r1 < 0 && r2 < 0) || (r1 > 0 && r2 > 0))
    {
        return 0;
    }

    double denom = a1 * b2 - a2 * b1;
    if (denom == 0)
    { // COLLINEAR
        return 0;
    }

    isect[0] = (b1 * c2 - b2 * c1) / denom;
    isect[1] = (a2 * c1 - a1 * c2) / denom;
    isect[2] = 0;

    return 1;    
}

// ****************************************************************************
// Method:    EdgeLineIsect
//
// Description:
//   Determines if the finit line specified by endpoints p1 and p2 intersects
//   any of the edges of the given cell.
//
// Returns:     1 if an intersection is found, 0 otherwise.A
//
// Arguments:
//   cell       The cell to test for intersection.
//   p1         The first endpoint of the finite line.
//   p2         The second endpoint of the finite line.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 27, 2004 
//
// ****************************************************************************
int
EdgeLineIsect(vtkCell *cell, const double *p1, const double *p2, double *x)
{
    double p3[3], p4[3];
    bool isectedEdge = false;
    int numEdges = cell->GetNumberOfEdges();
    int i;

    for (i = 0; i < numEdges && !isectedEdge; i++)
    {
        cell->GetEdge(i)->Points->GetPoint(0, p3);
        cell->GetEdge(i)->Points->GetPoint(1, p4);
        isectedEdge = (bool) LineLineIsect(p1, p2, p3, p4, x);
    }
    return (isectedEdge ? 1 : 0); 
}

// ****************************************************************************
// Method:    vtkVisItCellLocator::IntersectWithLine
//
// Description:
//   Return intersection points AND cellids of all cells intersected by
//   the finite line.
//
// Programmer:  Kathleen Bonnell
// Creation:    July 27, 2004 
//
// Modifications:
//   Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//   Rename ghost data array.
//
//   Kathleen Bonnell, Wed Oct 20 17:35:10 PDT 2004 
//   Return the cell-centers in pts instead of intersection points. 
//
//   Kathleen Bonnell, Thu May 19 11:34:05 PDT 2005 
//   Return both cell-centers AND intersection points. 
//
// ****************************************************************************
int vtkVisItCellLocator::IntersectWithLine(double a0[3], double a1[3], 
                                      vtkPoints *ipts, vtkPoints *cpts,
                                      vtkIdList *cells)
{
  this->TestCoPlanar = true;
  vtkGenericCell *cell = vtkGenericCell::New();

  double origin[3];
  double direction1[3];
  double direction2[3];
  double direction3[3];
  double hitPosition[3];
  double hitCellBoundsPosition[3], cellBounds[6];
  int hitCellBounds;
  double result;
  double bounds2[6];
  int i, leafStart, prod, loop;
  vtkIdType bestCellId = -1, cellId, cId;
  int idx;
  double tMax, dist[3];
  int npos[3];
  int pos[3];
  int bestDir, cellIsGhost;
  double stopDist, currDist;
  double length, maxLength = 0.0;
  double tempT, tempX[3], pc[3] = {0., 0., 0.};
  int tempId;
  vtkUnsignedCharArray *ghosts = 
    (vtkUnsignedCharArray *)this->DataSet->GetCellData()->GetArray("avtGhostZones");

  int ncells = this->DataSet->GetNumberOfCells();
  int ncellsTested = 0;
  // convert the line into i,j,k coordinates
  tMax = 0.0;
  for (i=0; i < 3; i++) 
    {
    direction1[i] = a1[i] - a0[i];
    length = this->Bounds[2*i+1] - this->Bounds[2*i];
    if (length > maxLength)
      {
      maxLength = length;
      }
    origin[i] = (a0[i] - this->Bounds[2*i]) / length; 
    direction2[i] = direction1[i] / length; 

    bounds2[2*i]   = 0.0;
    bounds2[2*i+1] = 1.0;
    tMax += direction2[i]*direction2[i];
    }
  stopDist = tMax*this->NumberOfDivisions;

  for (i = 0; i < 3; i++) 
    {
        direction3[i] = direction2[i]/tMax;
    }

  if (vtkBox::IntersectBox(bounds2, origin, direction2, hitPosition, result))
    {
    // start walking through the octants
    prod = this->NumberOfDivisions*this->NumberOfDivisions;
    leafStart = this->NumberOfOctants - this->NumberOfDivisions*prod;
    bestCellId = -1;
    
    // Clear the array that indicates whether we have visited this cell.
    // The array is only cleared when the query number rolls over.  This
    // saves a number of calls to memset.
    this->QueryNumber++;
    if (this->QueryNumber == 0)
      {
      this->ClearCellHasBeenVisited();
      this->QueryNumber++;    // can't use 0 as a marker
      }
    
    // set up curr and stop dist
    currDist = 0;
    for (i = 0; i < 3; i++)
      {
      currDist += (hitPosition[i] - origin[i])*(hitPosition[i] - origin[i]);
      }
    currDist = sqrt(currDist)*this->NumberOfDivisions;
    
    // add one offset due to the problems around zero
    for (loop = 0; loop <3; loop++)
      {
      hitPosition[loop] = hitPosition[loop]*this->NumberOfDivisions + 1.0;
      pos[loop] = (int)hitPosition[loop];
      // Adjust right boundary condition: if we intersect from the top, right,
      // or back; then pos must be adjusted to a valid octant index 
      if (pos[loop] > this->NumberOfDivisions)
        {
        pos[loop] = this->NumberOfDivisions;
        }
      }
    
    idx = leafStart + pos[0] - 1 + (pos[1] - 1)*this->NumberOfDivisions 
      + (pos[2] - 1)*prod;
    while ((bestCellId < 0) && (pos[0] > 0) && (pos[1] > 0) && (pos[2] > 0) &&
      (pos[0] <= this->NumberOfDivisions) &&
      (pos[1] <= this->NumberOfDivisions) &&
      (pos[2] <= this->NumberOfDivisions) &&
      (currDist < stopDist))
      {
      if (this->Tree[idx])
        {
        this->ComputeOctantBounds(pos[0]-1, pos[1]-1, pos[2]-1);
        for (tMax = VTK_DOUBLE_MAX, cellId=0; 
        cellId < this->Tree[idx]->GetNumberOfIds(); cellId++) 
          {
          cId = this->Tree[idx]->GetId(cellId);
          if (this->CellHasBeenVisited[cId] != this->QueryNumber)
            {
            ncellsTested++;
            this->CellHasBeenVisited[cId] = this->QueryNumber;
            cellIsGhost = 0;
            if (ghosts && this->IgnoreGhosts)
              {
              cellIsGhost = (int)ghosts->GetComponent(cId, 0); 
              }
            if (!cellIsGhost)
              {
              hitCellBounds = 0;
            
              // check whether we intersect the cell bounds
              if (this->CacheCellBounds)
                {
                hitCellBounds = vtkBox::IntersectBox(this->CellBounds[cId],
                                                 a0, direction1,
                                                 hitCellBoundsPosition, result);
                }
              else 
                {
                this->DataSet->GetCellBounds(cId, cellBounds);
                hitCellBounds = vtkBox::IntersectBox(cellBounds,
                                                 a0, direction1,
                                                 hitCellBoundsPosition, result);
                }
              if (hitCellBounds)
                {
                // now, do the expensive GetCell call and the expensive
                // intersect with line call
                this->DataSet->GetCell(cId, cell);
                if (IgnoreLines && cell->GetCellType() == VTK_LINE)     
                  continue;

                if (CellIntersectWithLine(cell, a0, a1, tempT, tempX, pc, tempId))
                  {
                  cells->InsertNextId(cId);
                  ipts->InsertNextPoint(tempX);
                  vtkVisItUtility::GetCellCenter(cell, tempX);
                  cpts->InsertNextPoint(tempX);
                  } // cell Isected line
                } // if (hitCellBounds)
              } // if !cellIsGhost
            } // if (!this->CellHasBeenVisited[cId])
          }
        }
      
      // move to the next octant
      tMax = VTK_DOUBLE_MAX;
      bestDir = 0;
      for (loop = 0; loop < 3; loop++)
        {
        if (direction3[loop] > 0)
          {
          npos[loop] = pos[loop] + 1;
          dist[loop] = (1.0 - hitPosition[loop] + pos[loop])/direction3[loop];
          if (dist[loop] == 0)
            {
            dist[loop] = 1.0/direction3[loop];
            }
          if (dist[loop] < 0)
            {
            dist[loop] = 0;
            }
          if (dist[loop] < tMax)
            {
            bestDir = loop;
            tMax = dist[loop];
            }
          }
        if (direction3[loop] < 0)
          {
          npos[loop] = pos[loop] - 1;
          dist[loop] = (pos[loop] - hitPosition[loop])/direction3[loop];
          if (dist[loop] == 0)
            {
            dist[loop] = -0.01/direction3[loop];
            }
          if (dist[loop] < 0)
            {
            dist[loop] = 0;
            }
          if (dist[loop] < tMax)
            {
            bestDir = loop;
            tMax = dist[loop];
            }
          }
        }
      // update our position
      for (loop = 0; loop < 3; loop++)
        {
        hitPosition[loop] += dist[bestDir]*direction3[loop];
        }
      currDist += dist[bestDir];
      // now make the move, find the smallest distance
      // only cross one boundry at a time
      pos[bestDir] = npos[bestDir];
      
      idx = leafStart + pos[0] - 1 + (pos[1]-1)*this->NumberOfDivisions + 
        (pos[2]-1)*prod;
      }
    }
  cell->Delete();   
  this->TestCoPlanar = false;
  return cells->GetNumberOfIds() > 0; 
}

// ****************************************************************************
//  Method:  vtkVisItCellLocator::FindClosestPointToLine
//
//  Purpose:
//    Return the point closest to the line along the pick ray, independent
//    of depth.  It is intended to help pick on point meshes.
//
//  Arguments:
//    a0,a1      points along the ray
//    dist2      (output) distance^2 from closest point to line
//    cellId     (output) cellid of closest point to line
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 17, 2005
//
// ****************************************************************************
void
vtkVisItCellLocator::FindClosestPointToLine(double a0[3], double a1[3],
                                            double &dist2, vtkIdType &cellId)
{
    int npoints = this->DataSet->GetNumberOfPoints();

    dist2 = VTK_DOUBLE_MAX;
    cellId = -1;

    for (vtkIdType p=0; p<npoints; p++)
    {
        double pt[3];
        this->DataSet->GetPoint(p, pt);

        // v0 is vector along ray
        double v0[3] = {a1[0]-a0[0], a1[1]-a0[1], a1[2]-a0[2]};

        // v1 is vector from some point on ray to this node
        double v1[3] = {a0[0]-pt[0], a0[1]-pt[1], a0[2]-pt[2]};

        // cross product
        double cp[3] = {v0[1]*v1[2] - v1[1]*v0[2],
                       v0[2]*v1[0] - v1[2]*v0[0],
                       v0[0]*v1[1] - v1[0]*v0[1]};

        // distance^2
        double dist_squared = ((cp[0]*cp[0] + cp[1]*cp[1] + cp[2]*cp[2]) /
                              (v0[0]*v0[0] + v0[1]*v0[1] + v0[2]*v0[2]));

        if (dist2 > dist_squared)
        {
            dist2 = dist_squared;
            cellId = p;
        }
    }
}
  
