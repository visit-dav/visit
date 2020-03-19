// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef VTK_OPENGL_TEXTURED_BACKGROUND_MAPPER_H
#define VTK_OPENGL_TEXTURED_BACKGROUND_MAPPER_H
#include <vtkMapper2D.h>

class vtkTexture;
class vtkRenderer;

// ****************************************************************************
// Class: vtkOpenGLTexturedBackgroundMapper
//
// Purpose:
//   OpenGL mapper class for internal use with vtkTexturedBackgroundActor.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:19:58 PST 2007
//
// Modifications:
//
// ****************************************************************************

// Not exposed in DLL on purpose
class vtkOpenGLTexturedBackgroundMapper : public vtkMapper2D
{
public:
  static vtkOpenGLTexturedBackgroundMapper *New();

  vtkTypeMacro(vtkOpenGLTexturedBackgroundMapper, vtkMapper2D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  void RenderOpaqueGeometry(vtkViewport *, vtkActor2D *) override;

  void SetTextureAndRenderers(vtkTexture *t, vtkRenderer *bg, vtkRenderer *canvas);
  void SetSphereMode(bool);
  void SetImageRepetitions(int,int);
protected:
   vtkOpenGLTexturedBackgroundMapper();
  ~vtkOpenGLTexturedBackgroundMapper();

  void DrawSphere(int X_RES, int Y_RES, float radius, const double *);
  void DrawImageSphere();
  void DrawImageFlat();

  vtkTexture  *tex;
  vtkRenderer *background;
  vtkRenderer *canvas;
  bool         sphereMode;
  int          imageRepeatX;
  int          imageRepeatY;
};

#endif
