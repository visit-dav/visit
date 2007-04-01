/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItCellDataToPointData.h,v $
  Language:  C++
  Date:      $Date: 2002/01/22 15:29:10 $
  Version:   $Revision: 1.17 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItCellDataToPointData - map cell data to point data
// .SECTION Description
// vtkVisItCellDataToPointData is a filter that transforms cell data (i.e., data
// specified per cell) into point data (i.e., data specified at cell
// points). The method of transformation is based on averaging the data
// values of all cells using a particular point. Optionally, the input cell
// data can be passed through to the output as well.

// .SECTION Caveats
// This filter is an abstract filter, that is, the output is an abstract type
// (i.e., vtkDataSet). Use the convenience methods (e.g.,
// vtkGetPolyDataOutput(), GetStructuredPointsOutput(), etc.) to get the type
// of output you want.

// .SECTION See Also
// vtkDataSetToDataSetFilter vtkPointData vtkCellData vtkPointDataToCellData


#ifndef __vtkVisItCellDataToPointData_h
#define __vtkVisItCellDataToPointData_h

#include "vtkDataSetToDataSetFilter.h"

class VTK_GRAPHICS_EXPORT vtkVisItCellDataToPointData : public vtkDataSetToDataSetFilter
{
public:
  static vtkVisItCellDataToPointData *New();
  vtkTypeRevisionMacro(vtkVisItCellDataToPointData,vtkDataSetToDataSetFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Control whether the input cell data is to be passed to the output. If
  // on, then the input cell data is passed through to the output; otherwise,
  // only generated point data is placed into the output.
  vtkSetMacro(PassCellData,int);
  vtkGetMacro(PassCellData,int);
  vtkBooleanMacro(PassCellData,int);

protected:
  vtkVisItCellDataToPointData();
  ~vtkVisItCellDataToPointData() {};

  void Execute();

  int PassCellData;
private:
  vtkVisItCellDataToPointData(const vtkVisItCellDataToPointData&);  // Not implemented.
  void operator=(const vtkVisItCellDataToPointData&);  // Not implemented.
};

#endif


