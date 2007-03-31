// .NAME vtkRectilinearGridFacelistFilter - get facelists for a rectilinear grid
// .SECTION Description
// vtkRectilinearGridFacelistFilter is a filter that extracts facelists from a
// rectilinear grid.

// .SECTION See Also
// vtkGeometryFilter

#ifndef __vtkRectilinearGridFacelistFilter_h
#define __vtkRectilinearGridFacelistFilter_h
#include <visit_vtk_exports.h>

#include "vtkRectilinearGridToPolyDataFilter.h"

class VISIT_VTK_API vtkRectilinearGridFacelistFilter : public vtkRectilinearGridToPolyDataFilter
{
public:
  vtkTypeMacro(vtkRectilinearGridFacelistFilter,vtkRectilinearGridToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a vtkRectilinearGridFacelistFilter.
  static vtkRectilinearGridFacelistFilter *New();

  // Description:
  // Set/Get ForceFaceConsolidation
  vtkSetMacro(ForceFaceConsolidation, int);
  vtkGetMacro(ForceFaceConsolidation, int);

protected:
  vtkRectilinearGridFacelistFilter();
  ~vtkRectilinearGridFacelistFilter() {};

  void Execute();

  int  ForceFaceConsolidation;

private:
  vtkRectilinearGridFacelistFilter(const vtkRectilinearGridFacelistFilter&);
  void operator=(const vtkRectilinearGridFacelistFilter&);

  void ConsolidationExecute(void);
};

#endif


