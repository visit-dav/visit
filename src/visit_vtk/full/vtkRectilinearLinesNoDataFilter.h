// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkRectilinearLinesNoDataFilter_h
#define __vtkRectilinearLinesNoDataFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

class     vtkIdTypeArray;

#include <vector>

// ****************************************************************************
//  Class:  vtkRectilinearLinesNoDataFilter
//
//  Purpose:
//    Create grid lines for a rectilinear grid.  It will not attempt to
//    copy any zonal scalar fields, since this makes no sense.  We do go
//    ahead and copy nodal scalars, though, since it sometimes does.
//    Also, no attempt has yet been made to avoid creating shared nodes
//    twice -- the benefit is small compared to the optimization this filter
//    this adds (instead to forcing it to go through the polydata route).
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 14, 2008
//
// ****************************************************************************

class VISIT_VTK_API vtkRectilinearLinesNoDataFilter :
  public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkRectilinearLinesNoDataFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Create a vtkRectilinearLinesNoDataFilter.
  static vtkRectilinearLinesNoDataFilter *New();

protected:
  vtkRectilinearLinesNoDataFilter();
  ~vtkRectilinearLinesNoDataFilter() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  vtkRectilinearLinesNoDataFilter(const vtkRectilinearLinesNoDataFilter&);
  void operator=(const vtkRectilinearLinesNoDataFilter&);
};

#endif
