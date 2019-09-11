// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkUnstructuredGridRelevantPointsFilter - strips away points and 
// associated point data that are not referenced by any cell in the input.
// associated with any cells 
// .SECTION Description
// vtkUnstructuredGridRelevantPointsFilter is a filter that takes unstructured
// grid data as input and generates unstructured grid data as output. 
// vtkUnstructuredGridRelevantPointsFilter removes points and associated
// point data that are not referenced by any cell in the input data set. 
//
// .SECTION Caveats
//
// .SECTION See Also
// vtkUnstructuredGridToUnstructuredGridFilter

#ifndef __vtkUnstructuredGridRelevantPointsFilter_h
#define __vtkUnstructuredGridRelevantPointsFilter_h
#include <visit_vtk_exports.h>

#include "vtkUnstructuredGridAlgorithm.h"

class VISIT_VTK_API vtkUnstructuredGridRelevantPointsFilter : 
  public vtkUnstructuredGridAlgorithm
{
public:
  vtkTypeMacro(vtkUnstructuredGridRelevantPointsFilter, 
               vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkUnstructuredGridRelevantPointsFilter *New();

protected:
  vtkUnstructuredGridRelevantPointsFilter(){};
 ~vtkUnstructuredGridRelevantPointsFilter(){};

  // Usual data generation method
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  vtkUnstructuredGridRelevantPointsFilter
       (const vtkUnstructuredGridRelevantPointsFilter&);
  void operator=(const vtkUnstructuredGridRelevantPointsFilter&);
};

#endif
