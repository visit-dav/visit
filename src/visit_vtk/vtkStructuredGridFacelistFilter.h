// .NAME vtkStructuredGridFacelistFilter - get facelists for a curvilinear grid
// .SECTION Description
// vtkStructuredGridFacelistFilter is a filter that extracts facelists from a
// curvilinear grid.

// .SECTION See Also
// vtkGeometryFilter

#ifndef __vtkStructuredGridFacelistFilter_h
#define __vtkStructuredGridFacelistFilter_h
#include <visit_vtk_exports.h>

#include "vtkStructuredGridToPolyDataFilter.h"

class VISIT_VTK_API vtkStructuredGridFacelistFilter : public vtkStructuredGridToPolyDataFilter
{
public:
  vtkTypeMacro(vtkStructuredGridFacelistFilter,vtkStructuredGridToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a vtkStructuredGridFacelistFilter.
  static vtkStructuredGridFacelistFilter *New();

protected:
  vtkStructuredGridFacelistFilter() {};
  ~vtkStructuredGridFacelistFilter() {};
  vtkStructuredGridFacelistFilter(const vtkStructuredGridFacelistFilter&) {};
  void operator=(const vtkStructuredGridFacelistFilter&) {};

  void Execute();
};

#endif


