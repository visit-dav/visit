// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_UNSTRUCTURED_GRID_BOUNDARY_FILTER_H
#define VTK_UNSTRUCTURED_GRID_BOUNDARY_FILTER_H
#include <visit_vtk_light_exports.h>

#include <vtkPolyDataAlgorithm.h>

class VISIT_VTK_LIGHT_API vtkUnstructuredGridBoundaryFilter :
    public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkUnstructuredGridBoundaryFilter, vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkUnstructuredGridBoundaryFilter *New();

  protected:
    vtkUnstructuredGridBoundaryFilter() {;};
    ~vtkUnstructuredGridBoundaryFilter() {;};
 
    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;
};

#endif
