#ifndef VTK_UNSTRUCTURED_GRID_BOUNDARY_FILTER_H
#define VTK_UNSTRUCTURED_GRID_BOUNDARY_FILTER_H
#include <vtkUnstructuredGridToPolyDataFilter.h>
#include <visit_vtk_exports.h>

class VISIT_VTK_API vtkUnstructuredGridBoundaryFilter 
    : public vtkUnstructuredGridToPolyDataFilter
{
  public:
    static vtkUnstructuredGridBoundaryFilter *New();
    vtkTypeMacro(vtkUnstructuredGridBoundaryFilter,
                 vtkUnstructuredGridToPolyDataFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

  protected:
    vtkUnstructuredGridBoundaryFilter() {;};
    ~vtkUnstructuredGridBoundaryFilter() {;};
 
    void Execute();
};

#endif
