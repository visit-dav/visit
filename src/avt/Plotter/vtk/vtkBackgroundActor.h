// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkBackgroundActor Create a gradient colored background.
// .SECTION Description
// vtkBackgroundActor draws a colored gradient background polygon over the
// entire viewport. This is a cheap special effect that makes images look nice.
// The colors of the gradient can be set for each of the corners of the image
// and the gradient pattern can also be set to radiate from the center of
// the viewport.
//
// .SECTION See Also
// vtkActor2D

#ifndef __vtkBackgroundActor_h
#define __vtkBackgroundActor_h
#include <plotter_exports.h>

#include "vtkActor2D.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include "vtkActor2D.h"
#include "vtkTimeStamp.h"

class PLOTTER_API vtkBackgroundActor : public vtkActor2D
{
public:
  vtkTypeMacro(vtkBackgroundActor, vtkActor2D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  // Description:
  // Instantiate a background actor.
  static vtkBackgroundActor *New();

  // Description:
  // Draw the actor as per the vtkProp superclass' API.
  int RenderOverlay(vtkViewport *) override;
  int RenderOpaqueGeometry(vtkViewport *) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *) override { return 0;}
  int HasTranslucentPolygonalGeometry() override { return 0;}

  // Description:
  // Sets the corner colors.
  void SetColors(const double colors[4][3]);
  void SetColor(int id, const double color[3]);

  // Description:
  // Sets the gradient fill mode. A value of 0 is top to bottom and a value
  // of 1 is radial.
  void SetGradientFillMode(int mode);
  vtkGetMacro(GradientFillMode, int);

  // Description:
  // Sets the number of radial rings used when doing the radial gradient mode.
  void SetNumRings(int rings);
  vtkGetMacro(NumRings, int);

  // Description:
  // Sets the number of radial steps used to do the radial gradient.
  void SetNumRadialSteps(int steps);
  vtkGetMacro(NumRadialSteps, int);
protected:
  vtkBackgroundActor();
  ~vtkBackgroundActor();

  void DestroyBackground();
  void CreateBackground();

  int                 GradientFillMode;
  double               GradientCoords[4];
  double               GradientColors[4][3];
  int                 NumRings;
  int                 NumRadialSteps;
  vtkPolyData         *GradientData;
  vtkActor2D          *GradientActor;
  vtkPolyDataMapper2D *GradientMapper;

private:
  vtkBackgroundActor(const vtkBackgroundActor&);
  void operator=(const vtkBackgroundActor&);
};


#endif
