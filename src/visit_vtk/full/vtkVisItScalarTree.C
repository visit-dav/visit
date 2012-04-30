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
    this->DataSet = NULL;
    this->MaxLevel = 20;
    this->BranchingFactor = 3;
    this->tree = NULL;
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
    if (this->tree)
        delete[] this->tree;
    if (this->DataSet != NULL)
        this->DataSet->UnRegister(this);
}

void
vtkVisItScalarTree::Initialize()
{
    if (this->tree)
        delete[] this->tree;
    this->tree = NULL;
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
//   Hank Childs, Wed Mar  9 07:28:55 PST 2005
//   Fix additional memory issues.  Tree created in slightly different manner
//   now.
//
//   Brad Whitlock, Tue Mar 13 13:55:36 PDT 2012
//   Added double implementation. Changed int to vtkIdType.
//
// ***************************************************************************

void
vtkVisItScalarTree::BuildTree()
{
    if (!this->DataSet)
    {
        vtkErrorMacro( << "No data to build tree with");
        return;
    }

    this->nCells = this->DataSet->GetNumberOfCells();
    if (this->nCells < 1)
    {
        vtkErrorMacro( << "No data to build tree with");
        return;
    }

    if (!this->DataSet->GetPointData()->GetScalars())
    {
        vtkErrorMacro( << "No scalar data to build tree with");
        return;
    }

    if ( this->tree != NULL && BuildTime > MTime && BuildTime > DataSet->GetMTime() )
        return;
    
    vtkDataArray *scalars = this->DataSet->GetPointData()->GetScalars();
    float *fscalars = (float *)scalars->GetVoidPointer(0);
    double *dscalars = (double *)scalars->GetVoidPointer(0);
    int stype = scalars->GetDataType();

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
    vtkIdType strideY = cell_dims[0];
    vtkIdType strideZ = cell_dims[0]*cell_dims[1];
    vtkIdType ptstrideY = pt_dims[0];
    vtkIdType ptstrideZ = pt_dims[0]*pt_dims[1];
    const vtkIdType X_val[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };
    const vtkIdType Y_val[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };
    const vtkIdType Z_val[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };

    if (this->tree)
        delete[] this->tree;
    
    vtkIdType numLeafs = (vtkIdType)ceil(double (nCells) / BranchingFactor);
    vtkIdType count = 1;
    for (this->levels = 0; count < numLeafs ; ++this->levels)
    {
        count *= this->BranchingFactor;
    }
   
    // If levels is too high, adjust appropriately.
    if (this->levels > this->MaxLevel)
    {
        this->levels = this->MaxLevel;
        this->bucketSize = (vtkIdType)ceil(double (nCells) /
                     pow((double)BranchingFactor, double(levels)));
    }
    else
        this->bucketSize = BranchingFactor;
    
    // Size of a full tree is a geometric series: (b^(n+1)-1) / (b-1)
    this->treeSize = (vtkIdType)((pow(double(BranchingFactor), double(levels + 1)) - 1)
                        / (BranchingFactor - 1));

    this->tree = new ScalarRange[this->treeSize];

    //
    // First, fill up the leaves of the tree.
    //
    this->leafOffset = this->treeSize - (vtkIdType)pow(double(this->BranchingFactor), double(this->levels));
    vtkIdList *lst = vtkIdList::New();
    for (vtkIdType t = 0 ; t < this->treeSize ; t++)
    {
        ScalarRange &leaf = this->tree[t];
        leaf.min = VTK_LARGE_FLOAT;
        leaf.max = -VTK_LARGE_FLOAT;
    }

    // Now find the min/max for each bucket.
    for (vtkIdType cellId = 0 ; cellId < this->nCells ; cellId++)
    {
        vtkIdType*pts;
        vtkIdType npts;
        vtkIdType arr8[8];
        
        // Get the points
        if (meshType==VTK_RECTILINEAR_GRID || meshType==VTK_STRUCTURED_GRID)
        {
            if (structured2D)
            {
                vtkIdType cellI = cellId % cell_dims[0];
                vtkIdType cellJ = cellId / strideY;

                for (vtkIdType j = 0 ; j < 4 ; ++j)
                {
                    arr8[j] = (cellI + X_val[j]) +
                              (cellJ + Y_val[j]) * ptstrideY; 
                } 

                pts = arr8;
                npts = 4;
            }
            else
            {
                vtkIdType cellI = cellId % cell_dims[0];
                vtkIdType cellJ = (cellId/strideY) % cell_dims[1];
                vtkIdType cellK = (cellId/strideZ);

                for (vtkIdType j = 0 ; j < 8 ; ++j)
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
            
        ScalarRange &leaf = this->tree[this->leafOffset + (cellId / this->bucketSize)];
        if(stype == VTK_FLOAT)
        {
            for (vtkIdType p = 0; p < npts; ++p)
            {
                double val = fscalars[pts[p]];
                if (val < leaf.min)
                    leaf.min = val;
                if (val > leaf.max)
                    leaf.max = val;
            }
        }
        else if(stype == VTK_DOUBLE)
        {
            for (vtkIdType p = 0; p < npts; ++p)
            {
                double val = dscalars[pts[p]];
                if (val < leaf.min)
                    leaf.min = val;
                if (val > leaf.max)
                    leaf.max = val;
            }
        }
        else
        {
            for (vtkIdType p = 0; p < npts; ++p)
            {
                double val = scalars->GetTuple1(pts[p]);
                if (val < leaf.min)
                    leaf.min = val;
                if (val > leaf.max)
                    leaf.max = val;
            }
        }
    }
    lst->Delete();


    //
    // Now propagate up the changes to the branches of the tree.
    //
    for (vtkIdType lev = this->levels - 1; lev >= 0; --lev)
    {
        //
        // The first index of each level is ((b ^ lev) - 1) / (b - 1)
        //
        vtkIdType offset;
        offset = (vtkIdType)((pow(double(this->BranchingFactor), double(lev)) - 1) /
                 (this->BranchingFactor - 1));

        vtkIdType len = (vtkIdType)pow(double(this->BranchingFactor), double(lev));


        vtkIdType cRow;
        cRow = (vtkIdType)((pow(double(this->BranchingFactor), double(lev + 1)) - 1)
                         / (this->BranchingFactor - 1));
        
        for (vtkIdType i = 0; i < len; ++i)
        {
            ScalarRange *parent = &(this->tree[offset + i]);
            for (vtkIdType j = 0 ; j < this->BranchingFactor ; j++)
            {
                const ScalarRange *child = &(this->tree[cRow + i * this->BranchingFactor + j]);
                if (child->min < parent->min)
                    parent->min = child->min;
                if (child->max > parent->max)
                    parent->max = child->max;
            }
        }
    }

    BuildTime.Modified();
}

void
vtkVisItScalarTree::GetCellList(double scalarValue, std::vector<vtkIdType> &v)
{
    BuildTree();
    
    if (!this->tree)
        return;
    
    this->searchValue = scalarValue;
    if (this->searchValue >= this->tree->min && this->searchValue <= this->tree->max)
        RecursiveSearch(v, 0, 0); 
}

void
vtkVisItScalarTree::RecursiveSearch(std::vector<vtkIdType> &v, vtkIdType index, vtkIdType lev)
{
    // Reached a leaf node
    if (lev == this->levels)
    {
        vtkIdType cId = (index - this->leafOffset) * this->bucketSize;
        for (vtkIdType i = 0; i < this->bucketSize && cId < this->nCells; ++i)
            v.push_back(cId++); 
        return;
    }

    vtkIdType childIndex = index * this->BranchingFactor + 1;
    for (vtkIdType i = 0; i < this->BranchingFactor; ++i)
    {
        vtkIdType ix = childIndex + i;
        if (this->searchValue >= this->tree[ix].min && this->searchValue <= this->tree[ix].max)
            RecursiveSearch(v, ix, lev + 1);
    }
}
