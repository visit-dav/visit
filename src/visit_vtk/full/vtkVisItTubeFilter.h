// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItTubeFilter_h
#define __vtkVisItTubeFilter_h

#include <vtkTubeFilter.h>
#include <visit_vtk_exports.h>

// Modifications:
//   Jeremy Meredith, Wed May 26 14:52:29 EDT 2010
//   Allow cell scalars for tube radius.
//
//   Kathleen Biagas, Tue Aug  7 10:55:50 PDT 2012
//   Added ScalarsForRadius var, so that a non-default scalar can be used
//   to scale the radius.
//
//   Kathleen Biagas, Mon July 20, 2026
//   Inherit from vtkTubeFilter. Add comments denoting VisIt additions.

class VISIT_VTK_API vtkVisItTubeFilter : public vtkTubeFilter
{
public:
  vtkTypeMacro(vtkVisItTubeFilter,vtkTubeFilter)
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkVisItTubeFilter *New();

  // begin VisIt addition
  vtkSetStringMacro(ScalarsForRadius);
  vtkGetStringMacro(ScalarsForRadius);
  // end VisIt addition

protected:
  vtkVisItTubeFilter();
  ~vtkVisItTubeFilter() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  // begin VisIt addition
  char *ScalarsForRadius;
  // end VisIt addition

  // Helper methods
  int GeneratePoints(vtkIdType offset, vtkIdType inCellId,
                     vtkIdType npts, const vtkIdType *pts,
                     vtkPoints *inPts, vtkPoints *newPts,
                     vtkPointData *pd, vtkPointData *outPD,
                     vtkFloatArray *newNormals,
                     vtkDataArray *inScalars, bool cellScalars,
                     double range[2], vtkDataArray *inVectors, double maxNorm,
                     vtkDataArray *inNormals);

  void GenerateTextureCoords(vtkIdType offset, vtkIdType npts,
                             const vtkIdType *pts,
                             vtkPoints *inPts,
                             vtkDataArray *inScalars, bool cellScalars,
                            vtkFloatArray *newTCoords);

private:
  vtkVisItTubeFilter(const vtkVisItTubeFilter&) = delete;
  void operator=(const vtkVisItTubeFilter&) = delete;
};

#endif
