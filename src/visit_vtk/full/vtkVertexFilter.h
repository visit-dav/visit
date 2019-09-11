// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkVertexFilter -- Make a vertex at each point incident to a cell
//     or at each cell.
//
// .SECTION Description
//     You can specify if you would like cell vertices or point vertices.
//

#ifndef __vtkVertexFilter_h
#define __vtkVertexFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

// ***************************************************************************
//  Class: vtkVertexFilter
//
//  Modifications:
//    Eric Brugger, Thu Jan 10 12:15:52 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

class VISIT_VTK_API vtkVertexFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkVertexFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetMacro(VertexAtPoints,bool);
  vtkGetMacro(VertexAtPoints,bool);
  vtkBooleanMacro(VertexAtPoints,bool);

  static vtkVertexFilter *New();

protected:
  vtkVertexFilter();
  ~vtkVertexFilter() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  bool VertexAtPoints;

private:
  vtkVertexFilter(const vtkVertexFilter&);
  void operator=(const vtkVertexFilter&);
};

#endif
