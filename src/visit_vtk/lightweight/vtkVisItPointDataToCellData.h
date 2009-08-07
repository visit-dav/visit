// Modifications:
//    Jeremy Meredith, Tue Aug  4 10:59:06 EDT 2009
//    No modifications other than renaming.  This file is exactly
//    the same as our vtk 5.0.0c source, except included here
//    to avoid linking the mdserver with the whole of VTK.


/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItPointDataToCellData.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItPointDataToCellData - map point data to cell data
// .SECTION Description
// vtkVisItPointDataToCellData is a filter that transforms point data (i.e., data
// specified per point) into cell data (i.e., data specified per cell).
// The method of transformation is based on averaging the data
// values of all points defining a particular cell. Optionally, the input point
// data can be passed through to the output as well.

// .SECTION Caveats
// This filter is an abstract filter, that is, the output is an abstract type
// (i.e., vtkDataSet). Use the convenience methods (e.g.,
// vtkGetPolyDataOutput(), GetStructuredPointsOutput(), etc.) to get the type
// of output you want.

// .SECTION See Also
// vtkPointData vtkCellData vtkCellDataToPointData

#ifndef __vtkVisItPointDataToCellData_h
#define __vtkVisItPointDataToCellData_h
#include <visit_vtk_light_exports.h>

#include "vtkDataSetAlgorithm.h"

class VISIT_VTK_LIGHT_API vtkVisItPointDataToCellData : public vtkDataSetAlgorithm
{
public:
  static vtkVisItPointDataToCellData *New();
  vtkTypeRevisionMacro(vtkVisItPointDataToCellData,vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Control whether the input point data is to be passed to the output. If
  // on, then the input point data is passed through to the output; otherwise,
  // only generated point data is placed into the output.
  vtkSetMacro(PassPointData,int);
  vtkGetMacro(PassPointData,int);
  vtkBooleanMacro(PassPointData,int);

protected:
  vtkVisItPointDataToCellData();
  ~vtkVisItPointDataToCellData() {};

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  int PassPointData;
private:
  vtkVisItPointDataToCellData(const vtkVisItPointDataToCellData&);  // Not implemented.
  void operator=(const vtkVisItPointDataToCellData&);  // Not implemented.
};

#endif


