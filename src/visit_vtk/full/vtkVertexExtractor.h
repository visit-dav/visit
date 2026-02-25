// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#ifndef vtkVertexExtractor_h
#define vtkVertexExtractor_h
#include <visit_vtk_exports.h>
#include <vtkPointSetAlgorithm.h>

// ***************************************************************************
//  Class: vtkVertexExtractor
//
//  Purpose: Extracts VTK_VERTEX and VTK_POLY_VERTEX cells from 
//           vtkPolyData or vtkUnstructuredGrid.
//
//  Notes:   VTK_POLY_VERTEX cells are converted to VTK_VERTEX so that output
//           can be passed to vtkOpenGLSphereMapper which does not handle
//           VTK_POLY_VERTEX cells.
//
// ***************************************************************************


class VISIT_VTK_API vtkVertexExtractor : public vtkPointSetAlgorithm
{
public:
  static vtkVertexExtractor *New();
  vtkTypeMacro(vtkVertexExtractor, vtkPointSetAlgorithm);

protected:
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int FillInputPortInformation(int port, vtkInformation *info) override;

  vtkVertexExtractor();
  ~vtkVertexExtractor() override;

private:
  vtkVertexExtractor(const vtkVertexExtractor&) = delete;
  void operator=(const vtkVertexExtractor&) = delete;
};

#endif
