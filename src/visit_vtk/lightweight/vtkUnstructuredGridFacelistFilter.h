#ifndef VTK_UNSTRUCTURED_GRID_FACELIST_FILTER_H
#define VTK_UNSTRUCTURED_GRID_FACELIST_FILTER_H
#include <vtkUnstructuredGridToPolyDataFilter.h>
#include <visit_vtk_light_exports.h>

class VISIT_VTK_LIGHT_API vtkUnstructuredGridFacelistFilter 
    : public vtkUnstructuredGridToPolyDataFilter
{
  public:
    static vtkUnstructuredGridFacelistFilter *New();
    vtkTypeMacro(vtkUnstructuredGridFacelistFilter,
                 vtkUnstructuredGridToPolyDataFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

  protected:
    vtkUnstructuredGridFacelistFilter() {;};
    ~vtkUnstructuredGridFacelistFilter() {;};
 
    void Execute();
};

#endif
