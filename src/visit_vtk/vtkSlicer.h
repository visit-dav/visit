/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicer.h,v $
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
// .NAME vtkSlicer - Slices vtkDataSets.


#ifndef __vtkSlicer_h
#define __vtkSlicer_h

#include <visit_vtk_exports.h>
#include "vtkDataSetToPolyDataFilter.h"


class VISIT_VTK_API vtkSlicer : public vtkDataSetToPolyDataFilter
{
public:
  vtkTypeRevisionMacro(vtkSlicer,vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with user-specified implicit function; initial value of 0.0; and
  // generating cut scalars turned off.
  static vtkSlicer *New();

  vtkGetVector3Macro(Normal, double);
  vtkSetVector3Macro(Normal, double);
  vtkGetVector3Macro(Origin, double);
  vtkSetVector3Macro(Origin, double);

  // Description:
  // Specify a cell list to cut against.  This allows outside modules to 
  // perform optimizations on which cells are cut.
  void SetCellList(int *, int);

protected:
  vtkSlicer();
  ~vtkSlicer();

  void Execute();
  void RectilinearGridExecute();
  void StructuredGridExecute();
  void UnstructuredGridExecute();
  void GeneralExecute();
  void SliceDataset(vtkDataSet *, vtkPolyData *);
  
  int *CellList;
  int  CellListSize;
  double Normal[3];
  double Origin[3];
private:
  vtkSlicer(const vtkSlicer&);  // Not implemented.
  void operator=(const vtkSlicer&);  // Not implemented.
};


#endif


