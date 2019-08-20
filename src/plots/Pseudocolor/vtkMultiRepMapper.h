// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef vtkMultiRepMapper_h
#define vtkMultiRepMapper_h


#include <vtkDataSetMapper.h>

// A Mapper that allows multiple representations (Surface, Wireframe, Points)
// to be drawn at the same time, via multi-pass renders.
//
class vtkMultiRepMapper : public vtkDataSetMapper
{
public:
  static vtkMultiRepMapper *New();
  vtkTypeMacro(vtkMultiRepMapper, vtkDataSetMapper)
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Render(vtkRenderer *ren, vtkActor *act) override;

  vtkSetMacro(DrawSurface, bool);
  vtkGetMacro(DrawSurface, bool);
  vtkBooleanMacro(DrawSurface, bool);

  vtkSetMacro(DrawWireframe, bool);
  vtkGetMacro(DrawWireframe, bool);
  vtkBooleanMacro(DrawWireframe, bool);

  vtkSetMacro(DrawPoints, bool);
  vtkGetMacro(DrawPoints, bool);
  vtkBooleanMacro(DrawPoints, bool);

  vtkSetVector3Macro(WireframeColor, double);
  vtkGetVector3Macro(WireframeColor, double);

  vtkSetVector3Macro(PointsColor, double);
  vtkGetVector3Macro(PointsColor, double);


protected:
  vtkMultiRepMapper();
 ~vtkMultiRepMapper();

  bool DrawSurface;
  bool DrawWireframe;
  bool DrawPoints;
  double WireframeColor[3];
  double PointsColor[3];
  bool currentScalarVis;

private:
  vtkMultiRepMapper(const vtkMultiRepMapper&) VTK_DELETE_FUNCTION;
  void operator=(const vtkMultiRepMapper&) VTK_DELETE_FUNCTION;
};

#endif
