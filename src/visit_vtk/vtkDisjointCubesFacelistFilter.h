#ifndef VTK_DISJOINT_CUBES_FACELIST_FILTER_H
#define VTK_DISJOING_CUBES_FACELIST_FILTER_H
#include <vtkUnstructuredGridToPolyDataFilter.h>
#include <visit_vtk_exports.h>

class VISIT_VTK_API vtkDisjointCubesFacelistFilter 
    : public vtkUnstructuredGridToPolyDataFilter
{
  public:
    static vtkDisjointCubesFacelistFilter *New();
    vtkTypeMacro(vtkDisjointCubesFacelistFilter,
                 vtkUnstructuredGridToPolyDataFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

  protected:
    vtkDisjointCubesFacelistFilter() {;};
    ~vtkDisjointCubesFacelistFilter() {;};
 
    void Execute();
};

#endif
