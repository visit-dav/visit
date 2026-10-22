// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkMergeConsecutivePolyDataPoints -- Merge matching consecutive
// points within PolyData primitives.
//
// .SECTION Description
// Loops over the points in the polydata primitives comparing the coordinates
// of consecutive points and assigning them the same point id if the
// coordinates match. The points will have the point id of the first of the
// matching points. It doesn't change the type of primitive or reduce the
// number of points in the primitive.
//

#ifndef __vtkMergeConsecutivePolyDataPoints_h
#define __vtkMergeConsecutivePolyDataPoints_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

// ***************************************************************************
//  Class: vtkMergeConsecutivePolyDataPoints
//
//  Modifications:
//
// ***************************************************************************

class VISIT_VTK_API vtkMergeConsecutivePolyDataPoints : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkMergeConsecutivePolyDataPoints, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate a filter that merges consecutive points if they match.
  static vtkMergeConsecutivePolyDataPoints *New();

protected:
  vtkMergeConsecutivePolyDataPoints();
  ~vtkMergeConsecutivePolyDataPoints() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  vtkMergeConsecutivePolyDataPoints(const vtkMergeConsecutivePolyDataPoints&);
  void operator=(const vtkMergeConsecutivePolyDataPoints&);
};

#endif
