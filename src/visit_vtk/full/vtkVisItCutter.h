// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItCutter_h
#define __vtkVisItCutter_h
#include <visit_vtk_exports.h>

#include <vtkCutter.h>

class VISIT_VTK_API vtkVisItCutter : public vtkCutter
{
public:
  vtkTypeMacro(vtkVisItCutter, vtkCutter)
  static vtkVisItCutter* New();


  // Off by default.
  // When set to true, forces use of vtkCutter::UnstructuredGridCutter
  // method. Useful for polygonal and polyhedral cells.
  // The vtkPlaneCutter path used by default in vtkCutter garbles
  // CellData for these cell types.
  vtkSetMacro(UnstructuredGridBypass, vtkTypeBool);
  vtkGetMacro(UnstructuredGridBypass, vtkTypeBool);
  vtkBooleanMacro(UnstructuredGridBypass, vtkTypeBool);

protected:
  vtkVisItCutter(vtkImplicitFunction* cf = nullptr);
  ~vtkVisItCutter() override;

  // override vtkCutter's method so that a different path can be
  // taken for UnstructuredGrids when 'UnstructuredGridBypass' is true.
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkTypeBool UnstructuredGridBypass;

private:
  vtkVisItCutter(const vtkVisItCutter&) = delete;
  void operator=(const vtkVisItCutter&) = delete;
};

#endif
