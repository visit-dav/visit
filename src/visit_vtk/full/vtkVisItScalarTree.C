/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItScalarTree.cxx,v $
  Language:  C++
  Date:      $Date: 2002/02/22 21:16:54 $
  Version:   $Revision: 1.66 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItScalarTree.h"

#include <vector>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

using std::vector;

vtkCxxRevisionMacro(vtkVisItScalarTree, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkVisItScalarTree);

vtkVisItScalarTree::vtkVisItScalarTree()
{
    DataSet = NULL;
    MaxLevel = 20;
    BranchingFactor = 3;
    tree = NULL;
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Mon Nov  1 09:55:59 PST 2004
//    Fixed memory leak.
//
// ****************************************************************************

vtkVisItScalarTree::~vtkVisItScalarTree()
{
    if (tree)
        delete[] tree;
    if (DataSet != NULL)
        DataSet->UnRegister(this);
}

void
vtkVisItScalarTree::Initialize()
{
    if (tree)
        delete[] tree;
    tree = NULL;
}

// ***************************************************************************
// Modifications:
//   Akira Haddox, Wed Jul 30 09:27:22 PDT 2003
//   Fixed a bug in optimization for 2D structured / rectlinear grids.
//
//   Eric Brugger, Tue Jul 27 07:59:13 PDT 2004
//   Add several casts to fix compile errors.
//
//   Hank Childs, Mon Aug  9 08:39:04 PDT 2004
//   Fix UMR.
//
// ***************************************************************************

void
vtkVisItScalarTree::BuildTree()
{
    if (!DataSet)
    {
        vtkErrorMacro( << "No data to build tree with");
        return;
    }

    nCells = DataSet->GetNumberOfCells();
    if (nCells < 1)
    {
        vtkErrorMacro( << "No data to build tree with");
        return;
    }

    if (!DataSet->GetPointData()->GetScalars())
    {
        vtkErrorMacro( << "No scalar data to build tree with");
        return;
    }

    if ( tree != NULL && BuildTime > MTime && BuildTime > DataSet->GetMTime() )
        return;
    
    float *scalars = (float *)(DataSet->GetPointData()->GetScalars()
                                    ->GetVoidPointer(0));

    //
    // Information for speeding up rectilinear and structured grids.
    // Based on code in vtkVisItContourFilter.
    //
    int meshType = DataSet->GetDataObjectType();
    
    int pt_dims[3] = { 1, 1, 1 };
    if (meshType == VTK_RECTILINEAR_GRID)
        ((vtkRectilinearGrid *)DataSet)->GetDimensions(pt_dims);
    else if (meshType == VTK_STRUCTURED_GRID)
        ((vtkStructuredGrid *)DataSet)->GetDimensions(pt_dims);

    //
    // Just to ensure we don't crash, 2D structured datasets
    // not in the XY plane we'll not optimize for.
    // 
    if ((meshType == VTK_RECTILINEAR_GRID ||
         meshType == VTK_STRUCTURED_GRID ) &&
        (pt_dims[0] == 1 || pt_dims[1] == 1))
    {
        meshType = VTK_POLY_DATA;
    }

    bool structured2D = false;
    if (meshType == VTK_RECTILINEAR_GRID || meshType == VTK_STRUCTURED_GRID)
        structured2D = (pt_dims[2] == 1);

    int cell_dims[3] = { 0, 0, 0 };
    cell_dims[0] = pt_dims[0] - 1;
    cell_dims[1] = pt_dims[1] - 1;
    cell_dims[2] = pt_dims[2] - 1;
    int strideY = cell_dims[0];
    int strideZ = cell_dims[0]*cell_dims[1];
    int ptstrideY = pt_dims[0];
    int ptstrideZ = pt_dims[0]*pt_dims[1];
    const int X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    const int Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    const int Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };

    if (tree)
        delete[] tree;
    
    int numLeafs = (int)ceil(double (nCells) / BranchingFactor);
    int count = 1;
    for (levels = 0; count < numLeafs ; ++levels)
    {
        count *= BranchingFactor;
    }
   
    // If levels is too high, adjust appropriately.
    if (levels > MaxLevel)
    {
        levels = MaxLevel;
        bucketSize = (int)ceil(double (nCells) /
                     pow((double)BranchingFactor, levels));
    }
    else
        bucketSize = BranchingFactor;
    
    // Size of a full tree is a geometric series: (b^(n+1)-1) / (b-1)
    treeSize = (int)((pow((double)BranchingFactor, levels + 1) - 1)
                        / (BranchingFactor - 1));

    tree = new ScalarRange[treeSize];

    //
    // First, fill up the leaves of the tree.
    //
    
    leafOffset = treeSize - (int)pow((double)BranchingFactor, levels);
    int cellId = 0;
    vtkIdList *lst = vtkIdList::New();
    while (cellId < nCells)
    {
        ScalarRange &leaf = tree[leafOffset + (cellId / bucketSize)];

        leaf.min = VTK_LARGE_FLOAT;
        leaf.max = -VTK_LARGE_FLOAT;

        int i;
        for (i = 0; i < bucketSize && cellId < nCells; ++i)
        {
            int *pts;
            int npts;
            int arr8[8];
            
            // Get the points
            if (meshType == VTK_RECTILINEAR_GRID 
                 || meshType == VTK_STRUCTURED_GRID)
            {
                if (structured2D)
                {
                    int cellI = cellId % cell_dims[0];
                    int cellJ = cellId / strideY;

                    int j;
                    for (j = 0 ; j < 4 ; ++j)
                    {
                        arr8[j] = (cellI + X_val[j]) +
                                  (cellJ + Y_val[j]) * ptstrideY; 
                    } 

                    pts = arr8;
                    npts = 4;
                }
                else
                {
                    int cellI = cellId % cell_dims[0];
                    int cellJ = (cellId/strideY) % cell_dims[1];
                    int cellK = (cellId/strideZ);
                    int j;
                    for (j = 0 ; j < 8 ; ++j)
                    {
                        arr8[j] = (cellI + X_val[j]) +
                                  (cellJ + Y_val[j])*ptstrideY 
                                    + (cellK + Z_val[j])*ptstrideZ;
                    }
                    pts = arr8;
                    npts = 8;
                }
            } 
            else
            { 
                DataSet->GetCellPoints(cellId, lst);
                npts = lst->GetNumberOfIds();
                pts = lst->GetPointer(0);
            }
            
            int p;
            for (p = 0; p < npts; ++p)
            {
                float val = scalars[pts[p]];
                if (val < leaf.min)
                    leaf.min = val;
                if (val > leaf.max)
                    leaf.max = val;
            }
            
            ++cellId;
        }
    }
    lst->Delete();


    //
    // Now propegate up the changes to the branches of the tree.
    //
    int lev;
    for (lev = levels - 1; lev >= 0; --lev)
    {
        //
        // The first index of each level is ((b ^ lev) - 1) / (b - 1)
        //
        int offset;
        offset = (int)((pow((double)BranchingFactor, lev) - 1) /
                 (BranchingFactor - 1));
        int cRow;
        cRow = (int)((pow((double)BranchingFactor, lev + 1) - 1)
                         / (BranchingFactor - 1));
        
        int len = (int)pow((double)BranchingFactor, lev);

        int i;
        for (i = 0; i < len; ++i)
        {
            ScalarRange *parent = &tree[offset + i];
            ScalarRange *child = &(tree[cRow + i * BranchingFactor]);
            
            parent->min = child->min;
            parent->max = child->max;
            ++child;
            int j;
            for (j = 0; j < 4; ++j)
            {
                if (child->min < parent->min)
                    parent->min = child->min;
                if (child->max > parent->max)
                    parent->max = child->max;
                ++child;
            }
        }
    }

    BuildTime.Modified();
}

void
vtkVisItScalarTree::GetCellList(float scalarValue, std::vector<int> &v)
{
    BuildTree();
    
    if (!tree)
        return;
    
    searchValue = scalarValue;
    if (searchValue >= tree->min && searchValue <= tree->max)
        RecursiveSearch(v, 0, 0); 
}

void
vtkVisItScalarTree::RecursiveSearch(std::vector<int> &v, int index, int lev)
{
    // Reached a leaf node
    if (lev == levels)
    {
        int cId = (index - leafOffset) * bucketSize;
        int i;
        for (i = 0; i < bucketSize && cId < nCells; ++i)
            v.push_back(cId++); 
        return;
    }

    int childIndex = index * BranchingFactor + 1;
    int i;
    for (i = 0; i < BranchingFactor; ++i)
    {
        int ix = childIndex + i;
        if (searchValue >= tree[ix].min && searchValue <= tree[ix].max)
            RecursiveSearch(v, ix, lev + 1);
    }
}
