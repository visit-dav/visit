/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItContourFilter.h,v $
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
// .NAME vtkVisItContourFilter - Contours vtkDataSets.


#ifndef __vtkVisItContourFilter_h
#define __vtkVisItContourFilter_h

#include <visit_vtk_exports.h>
#include "vtkDataSetToPolyDataFilter.h"


class VISIT_VTK_API vtkVisItContourFilter : public vtkDataSetToPolyDataFilter
{
public:
  vtkTypeRevisionMacro(vtkVisItContourFilter,vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with user-specified implicit function; initial value of 0.0; and
  // generating cut scalars turned off.
  static vtkVisItContourFilter *New();

  // Description:
  // Set/Get isovalue.
  vtkSetMacro(Isovalue,float);
  vtkGetMacro(Isovalue,float);
  
  // Description:
  // Specify a cell list to cut against.  This allows outside modules to 
  // perform optimizations on which cells are cut.
  void SetCellList(int *, int);

protected:
  vtkVisItContourFilter();
  ~vtkVisItContourFilter();

  void Execute();
  void RectilinearGridExecute();
  void StructuredGridExecute();
  void UnstructuredGridExecute();
  void GeneralExecute();
  void ContourDataset(vtkDataSet *, vtkPolyData *);
  
  float Isovalue;

  int *CellList;
  int  CellListSize;

  float *GetPointScalars(void);
  
private:
  vtkVisItContourFilter(const vtkVisItContourFilter&);  // Not implemented.
  void operator=(const vtkVisItContourFilter&);  // Not implemented.
};


#endif


