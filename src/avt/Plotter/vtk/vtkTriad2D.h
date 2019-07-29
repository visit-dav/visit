// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkTriad2D Create a triad showing the x-y-z axes for the current view.
// .SECTION Description
// vtkTriad2D draws the x, y, and z unit vectors on the screen and updates them
// every time the camera changes.
// The triad's size stays constant despite changes in the camera location and
// is overlayed over all 3D actors.
// You may set the location of the origin of the triad through SetOrigin and
// may set the length of the unit vectors with SetAxisLength.
// You may also set the properties of the axes by getting each access and
// setting the properties through its (vtkVisItAxisActor2D) methods.
//
// .SECTION See Also
// vtkActor2D vtkVisItAxisActor2D

#ifndef __vtkTriad2D_h
#define __vtkTriad2D_h
#include <plotter_exports.h>

#include "vtkActor2D.h"
#include "vtkVisItAxisActor2D.h"
#include "vtkCamera.h"


class PLOTTER_API vtkTriad2D : public vtkActor2D
{
public:
  vtkTypeMacro(vtkTriad2D, vtkActor2D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  // Description:
  // Instantiate a triad with origin at (0.1, 0.1) in normalized viewport 
  // coordinates, with a unit vector length of 0.8.  
  static vtkTriad2D *New();

  // Description:
  // Draw the triad as per the vtkProp superclass' API.
  int RenderOverlay(vtkViewport *) override;
  int RenderOpaqueGeometry(vtkViewport *) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override { return 0;}
  int HasTranslucentPolygonalGeometry() override { return 0;}

  // Description:
  // Set the origin of the triad in normalized viewport coordinates.
  void SetOrigin(float new_origin[2]);
  void SetOrigin(float origin_x, float origin_y);
  vtkGetVector2Macro(Origin, float);

  // Description:
  // Set the length of the unit vector in normalized viewport coordinates.
  vtkSetClampMacro(AxisLength, float, 0., 1.);
  vtkGetMacro(AxisLength, float);

  // Description:
  // Set/Get the camera to perform scaling and translation of the
  // vtkCubeAxesActor2D.
  virtual void SetCamera(vtkCamera*);
  vtkGetObjectMacro(Camera,vtkCamera);

  // Description:
  // Allow for axes to be changed directly - changing colors and such.  
  vtkGetObjectMacro(XAxis, vtkVisItAxisActor2D);
  vtkGetObjectMacro(YAxis, vtkVisItAxisActor2D);
  vtkGetObjectMacro(ZAxis, vtkVisItAxisActor2D);

protected:
  vtkTriad2D();
  ~vtkTriad2D();

  vtkCamera       *Camera;

  float            Origin[2];
  float            AxisLength;

  vtkVisItAxisActor2D  *XAxis, *YAxis, *ZAxis;

  void              TranslateVectorToViewport(const float world_vector[4], 
                                              float vport[2]);

private:
  vtkTriad2D(const vtkTriad2D&);
  void operator=(const vtkTriad2D&);
};


#endif
