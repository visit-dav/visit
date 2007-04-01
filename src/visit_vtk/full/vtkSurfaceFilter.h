
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

#include <vtkDataSetToUnstructuredGridFilter.h>
#include <vtkFloatArray.h>
#include <vtkSetGet.h>

class vtkPointSet;
class vtkRectilinearGrid;

//=======================================================================
// Modifications:
//   Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//   vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//   Use ObjectMacro instead of plain Macro.
//=======================================================================

class VISIT_VTK_API  
vtkSurfaceFilter : public vtkDataSetToUnstructuredGridFilter
{
public:
  static vtkSurfaceFilter *New();
  vtkTypeMacro(vtkSurfaceFilter,vtkDataSetToUnstructuredGridFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the scalars to use for z-values in the output. 
  virtual void SetinScalars(vtkFloatArray*); 
  vtkGetObjectMacro(inScalars, vtkFloatArray); 

 
protected:
  vtkSurfaceFilter();
  ~vtkSurfaceFilter() ;

  void Execute();
  void ExecuteRectilinearGrid(vtkRectilinearGrid*);
  void ExecutePointSet(vtkPointSet*);

// Protected Data Members

  vtkFloatArray *inScalars;

private:
  vtkSurfaceFilter(const vtkSurfaceFilter&);
  void operator=(const vtkSurfaceFilter&);
};

#endif


