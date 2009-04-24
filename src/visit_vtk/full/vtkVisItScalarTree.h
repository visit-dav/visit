/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItScalarTree.h,v $
  Language:  C++
  Date:      $Date: 2002/01/22 15:29:13 $
  Version:   $Revision: 1.54 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItScalarTree - Takes in point data from a dataset and
// organizes the range of values a cell takes into a tree, which can
// return what cells may potentially contain a value. Used for
// accelerating contouring code.

#ifndef __vtkVisItScalarTree_h
#define __vtkVisItScalarTree_h

#include <visit_vtk_exports.h>
#include "vtkDataSet.h"
#include "vtkObject.h"

#include <vector>

class VISIT_VTK_API vtkVisItScalarTree : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkVisItScalarTree,vtkObject);

  // Description:
  // Instantiate scalar tree with maximum level of 20 and branching
  // factor of 3.
  static vtkVisItScalarTree *New();

  // Description:
  // Build the tree from the points/cells defining this dataset.
  vtkSetObjectMacro(DataSet,vtkDataSet);
  vtkGetObjectMacro(DataSet,vtkDataSet);

  // Description:
  // Set the branching factor for the tree. This is the number of
  // children per tree node. Smaller values (minimum is 2) mean deeper
  // trees and more memory overhead. Larger values mean shallower
  // trees, less memory usage, but worse performance.
  vtkSetClampMacro(BranchingFactor,int,2,VTK_LARGE_INTEGER);
  vtkGetMacro(BranchingFactor,int);

  // Description:
  // Set the maximum allowable level for the tree. 
  vtkSetClampMacro(MaxLevel,int,1,VTK_LARGE_INTEGER);
  vtkGetMacro(MaxLevel,int);

  // Description:
  // Construct the scalar tree from the dataset provided.
  // Checks modified time stamp to see if tree needs regenerating.
  void BuildTree();
  
  // Description:
  // Initialize locator. Frees memory and resets object as appropriate.
  void Initialize();

  // Description:
  // Fill a vector of cells that may contain a particular scalar
  // value. Automatically calls BuildTree.
  void GetCellList(float scalarValue, std::vector<int> &v);

  // Description:
  // Free the allocated tree structure.
  void FreeSearchStructure()
  {
      if (tree)
          delete tree;
      tree = NULL;
  }

protected:
  vtkVisItScalarTree();
  ~vtkVisItScalarTree();
  vtkVisItScalarTree(const vtkVisItScalarTree&);  // Not implemented.
  void operator=(const vtkVisItScalarTree&); // Not implemented.

  void RecursiveSearch(std::vector<int> &v, int index, int lev);

  vtkTimeStamp BuildTime;
  
  vtkDataSet *DataSet;
  int levels;
  int bucketSize;
  int nCells;
  int leafOffset;

  int MaxLevel;
  int BranchingFactor;

  struct ScalarRange
  {
      float min, max;
  };
  
  ScalarRange *tree;
  int treeSize;

  float searchValue;
};


#endif


