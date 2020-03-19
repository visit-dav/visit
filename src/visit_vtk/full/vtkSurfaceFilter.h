// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

//========================================================================
//
//  Program:   VisIt
//  Module:    $RCSfile: vtkSurfaceFilter.h,v $
//  Language:  C++
//  Date:      $Date: 2000/09/20 18:11:07 $
//  Version:   $Revision: 1.24 $
//
//
//
//=========================================================================
// .NAME vtkSurfaceFilter - 
// .SECTION Description
// vtkSurfaceFilter is a filter object that, 
//
// Output is DataSet, same concrete type as input except for inputs
// of rectilinear grids which are converted to unstructured grids for output.
//
// .SECTION See Also
//
// .SECTION Caveats
//

#ifndef __vtkSurfaceFilter_h
#define __vtkSurfaceFilter_h
#include <visit_vtk_exports.h>

#include <vtkUnstructuredGridAlgorithm.h>

class vtkDataAray;
class vtkPointSet;
class vtkRectilinearGrid;

//=======================================================================
// Modifications:
//   Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//   vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//   Use ObjectMacro instead of plain Macro.
//=======================================================================

class VISIT_VTK_API  
vtkSurfaceFilter : public vtkUnstructuredGridAlgorithm
{
public:
  vtkTypeMacro(vtkSurfaceFilter,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkSurfaceFilter *New();

  // Description:
  // Set/Get the scalars to use for z-values in the output. 
  virtual void SetinScalars(vtkDataArray*); 
  vtkGetObjectMacro(inScalars, vtkDataArray); 
 
protected:
  vtkSurfaceFilter();
  ~vtkSurfaceFilter() ;

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  void ExecuteRectilinearGrid(vtkRectilinearGrid *, vtkUnstructuredGrid *);
  void ExecutePointSet(vtkPointSet *, vtkUnstructuredGrid *);

// Protected Data Members

  vtkDataArray *inScalars;

private:
  vtkSurfaceFilter(const vtkSurfaceFilter&);
  void operator=(const vtkSurfaceFilter&);
};

#endif
