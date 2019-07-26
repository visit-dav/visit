// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkVisItScalarBarWithOpacityActor - Create a scalar bar with labels, title and
// range
// .SECTION Description
// vtkVisItScalarBarWithOpacityActor creates a scalar bar with annotation text. A scalar
// bar is a legend that indicates to the viewer the correspondence between
// color value and data value. The legend consists of a rectangular bar
// made of rectangular pieces each colored a constant value. Since
// vtkVisItScalarBarWithOpacityActor is a subclass of vtkActor2D, it is drawn in the
// image plane (i.e., in the renderer's viewport) on top of the 3D graphics
// window.
//
// To use vtkVisItScalarBarWithOpacityActor you must associate a vtkScalarsToColors (or
// subclass) with it. The lookup table defines the colors and the
// range of scalar values used to map scalar data.  Typically, the
// number of colors shown in the scalar bar is not equal to the number
// of colors in the lookup table, in which case sampling of
// the lookup table is performed.
//
// Other optional capabilities include specifying the fraction of the
// viewport size (both x and y directions) which will control the size
// of the scalar bar, the number of annotation labels, and the font
// attributes of the annotation text. The actual position of the
// scalar bar on the screen is controlled by using the
// vtkActor2D::SetPosition() method (by default the scalar bar is
// position on the right side of the viewport).  Other features include
// the ability control the format (print style) with which to print the
// labels on the scalar bar. Also, the vtkVisItScalarBarWithOpacityActor's property
// is applied to the scalar bar and annotation (including color, layer, and
// compositing operator).

// .SECTION See Also
// vtkActor2D vtkTextMapper vtkPolyDataMapper2D vtkVisItScalarBarActor

#ifndef __vtkVisItScalarBarWithOpacityActor_h
#define __vtkVisItScalarBarWithOpacityActor_h
#include <plotter_exports.h>
#include <vtkVisItScalarBarActor.h>

class PLOTTER_API vtkVisItScalarBarWithOpacityActor : public vtkVisItScalarBarActor
{
public:
  vtkTypeMacro(vtkVisItScalarBarWithOpacityActor,vtkVisItScalarBarActor);

  // Description:
  // Instantiate object.
  static vtkVisItScalarBarWithOpacityActor *New();

  void SetLegendOpacities(const unsigned char *opacity);

protected:
  vtkVisItScalarBarWithOpacityActor();
  virtual ~vtkVisItScalarBarWithOpacityActor();

  void BuildColorBar(vtkViewport *) override;

  unsigned char opacities[256];

private:
  vtkVisItScalarBarWithOpacityActor(const vtkVisItScalarBarWithOpacityActor&);
  void operator=(const vtkVisItScalarBarWithOpacityActor&);
};

#endif
