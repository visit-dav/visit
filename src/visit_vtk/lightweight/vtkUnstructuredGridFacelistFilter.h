// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_UNSTRUCTURED_GRID_FACELIST_FILTER_H
#define VTK_UNSTRUCTURED_GRID_FACELIST_FILTER_H
#include <visit_vtk_light_exports.h>

#include <vtkPolyDataAlgorithm.h>

//
// MCMiller-11Jan19: My understanding of what this class does, from a cursory
// review of the code, is that it iterates over all cells in the mesh (grid)
// adding the faces from each cell to various hashes. However, the face-adding
// logic is designed to add a face only if it has never been seen before
// and otherwise remove the face. For topological reasons, faces are shared by
// at most two cells. So a face will only ever be encountered once or twice.
// Once all faces from all cells have been iterated what remains are those
// faces that were only ever encountered once. Those are the *external* faces
// of the input mesh (grid). There is no logic here to address whether the
// cells involved (or their corresonding nodes) are associated with any
// ghosting.
//
// Note 1: a similar operation is performed in the Silo plugin to handle
// painting of enumerated scalar nodal values for annotation int zonelists
// except that multi-layer STL maps are used for the hashing.
//
// Note 2: This kind of algorithm is exactly the kind of thing something
// like Big Data Apache Spark filters handle very well.
//

class VISIT_VTK_LIGHT_API vtkUnstructuredGridFacelistFilter :
    public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkUnstructuredGridFacelistFilter, vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkUnstructuredGridFacelistFilter *New();

  protected:
    vtkUnstructuredGridFacelistFilter() {;};
    ~vtkUnstructuredGridFacelistFilter() {;};
 
    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;
};

#endif
