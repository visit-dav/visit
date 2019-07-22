// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkStructuredGridFacelistFilter - get facelists for a curvilinear grid
// .SECTION Description
// vtkStructuredGridFacelistFilter is a filter that extracts facelists from a
// curvilinear grid.

// .SECTION See Also
// vtkGeometryFilter

#ifndef __vtkStructuredGridFacelistFilter_h
#define __vtkStructuredGridFacelistFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

class VISIT_VTK_API vtkStructuredGridFacelistFilter :
  public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkStructuredGridFacelistFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Create a vtkStructuredGridFacelistFilter.
  static vtkStructuredGridFacelistFilter *New();

protected:
  vtkStructuredGridFacelistFilter() {};
  ~vtkStructuredGridFacelistFilter() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  vtkStructuredGridFacelistFilter(const vtkStructuredGridFacelistFilter&); // not implemented
  void operator=(const vtkStructuredGridFacelistFilter&); // not implemented
};

#endif
