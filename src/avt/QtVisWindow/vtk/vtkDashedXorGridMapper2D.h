// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// Based upon vtkRubberBandMapper2D. Draws horizontal and vertical dashed
// lines. For a given line, pixels are placed on a 1D grid to determine
// where the dashes are. Thus, drawing a line from [0, 10], [10, 20] has
// the same effect as drawing from [0, 20], or [20, 0].
// When drawing a single pixel, use SetHorizontalBias to say whether you
// want to use the horizontal or vertical grid.

#ifndef __vtkDashedXorGridMapper2D_h
#define __vtkDashedXorGridMapper2D_h
#include <qtviswindow_exports.h>

#include "vtkPolyDataMapper2D.h"

class QWidget;
struct vtkDashedXorGridMapper2DPrivate;

// ***************************************************************************
//  Modifications:
//    Alister Maguire, Thu Jun 24 10:57:30 PDT 2021
//    Changed the name of class variable "d" to "privateInstance".
//
// ***************************************************************************
class QTVISWINDOW_API vtkDashedXorGridMapper2D : public vtkPolyDataMapper2D
{
public:
  vtkTypeMacro(vtkDashedXorGridMapper2D,vtkPolyDataMapper2D);
  static vtkDashedXorGridMapper2D *New();

  // Description:
  // Set the widget over which the drawing will happen.
  void SetWidget(QWidget *widget);

  // Description:
  // Actually draw the poly data.
  void RenderOverlay(vtkViewport* viewport, vtkActor2D* actor) override;

  // Description:
  // Release graphics resources.
  void ReleaseGraphicsResources(vtkWindow *) override;

  // Description:
  // Sets length of dashes and spaces. Call before first use.
  void SetDots(int drawn, int spaced);

  // Description:
  // In the case of drawing a point, it will use the horizontal grid
  // if the hb is set, otherwise it will use the vertical grid.
  void SetHorizontalBias(bool hb)
  { horizontalBias = hb; }

protected:
  void RenderOverlay_X11(vtkViewport* viewport, vtkActor2D* actor);
  void RenderOverlay_Qt(vtkViewport* viewport, vtkActor2D* actor);

  int pixelDrawn, pixelSpaced;
  bool horizontalBias;

  vtkDashedXorGridMapper2DPrivate *privateInstance;

  vtkDashedXorGridMapper2D();
  ~vtkDashedXorGridMapper2D();

  // Returns whether a point in is on a dashed line.
  bool IsDash(int x)
  {
    return ((x % (pixelDrawn + pixelSpaced)) <= pixelDrawn);
  }

  bool IsBeginningDash(int x)
  {
    return (x % (pixelDrawn + pixelSpaced) == 0);
  }

  // Moves forward to the next dash from a given point. Point does not
  // have to be on a dash.
  int NextDash(int x)
  {
    return (x + pixelDrawn + pixelSpaced - (x % (pixelDrawn + pixelSpaced)));
  }

private:
  vtkDashedXorGridMapper2D(const vtkDashedXorGridMapper2D&);
  void operator=(const vtkDashedXorGridMapper2D&);

};


#endif

