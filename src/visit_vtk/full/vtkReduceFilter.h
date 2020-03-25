// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkReduceFilter -- Reduce the number of tensors.
//
// .SECTION Description
// Allows a dataset to be reduced by keeping only one out of every N points.
// It takes an input dataset and throws away some points making poly data that
// can go into vtkGlyph3D.
//
// .CAVEATS You can specify the stride in one of two ways -- by specifying how
//  many total elements you want (SetNumberOfElements) or by specifying how
//  many to process for every one saved (SetStride).
//

#ifndef __vtkReduceFilter_h
#define __vtkReduceFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

// ***************************************************************************
//  Class: vtkReduceFilter
//
//  Modifications:
//    Eric Brugger, Thu Jan 10 12:02:36 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

class VISIT_VTK_API vtkReduceFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkReduceFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate a stride filter that throws away nine of every ten elements.
  static vtkReduceFilter *New();

  void ReduceVectors();
  void ReduceTensors();
  void SetStride(int);
  void SetNumberOfElements(int);
  void SetLimitToOriginal(bool);

protected:
  vtkReduceFilter();
  ~vtkReduceFilter() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  enum ReduceType
  {
    rVectors,
    rTensors,
  };

  ReduceType reduceType;

  int stride;
  int numEls;
  bool origOnly;

private:
  vtkReduceFilter(const vtkReduceFilter&);
  void operator=(const vtkReduceFilter&);
};

#endif
