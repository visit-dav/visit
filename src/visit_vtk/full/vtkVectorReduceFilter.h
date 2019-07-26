// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkVectorReduceFilter -- Reduce the number of vectors.
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


#ifndef __vtkVectorReduceFilter_h
#define __vtkVectorReduceFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

// ***************************************************************************
//  Class: vtkVectorReduceFilter
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  8 11:07:57 EDT 2008
//    Added ability to limit to only one output vector per original
//    cell/point.  Also, fixed cell-based vector algorithm bugs.
//
//    Eric Brugger, Thu Jan 10 10:07:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

class VISIT_VTK_API vtkVectorReduceFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkVectorReduceFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate a stride filter that throws away nine of every ten elements.
  static vtkVectorReduceFilter *New();

  void SetStride(int);
  void SetNumberOfElements(int);
  void SetLimitToOriginal(bool);

protected:
  vtkVectorReduceFilter();
  ~vtkVectorReduceFilter() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  int stride;
  int numEls;
  bool origOnly;

private:
  vtkVectorReduceFilter(const vtkVectorReduceFilter&);
  void operator=(const vtkVectorReduceFilter&);
};

#endif
