// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_TEXTURED_BACKGROUND_ACTOR_H
#define VTK_TEXTURED_BACKGROUND_ACTOR_H
#include <vtkActor2D.h>
#include <plotter_exports.h>

class vtkTexture;
class vtkRenderer;

// ****************************************************************************
// Class: vtkTexturedBackgroundActor
//
// Purpose:
//   This class is a 2D actor that can draw a background image into a render
//   window.
//
// Notes:      This class does not need to be associated with a mapper because
//             a specialized mapper is created and used internally.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:21:51 PST 2007
//
// Modifications:
//
// ****************************************************************************

class PLOTTER_API vtkTexturedBackgroundActor : public vtkActor2D
{
public:
  static vtkTexturedBackgroundActor *New();

  vtkTypeMacro(vtkTexturedBackgroundActor, vtkActor2D);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  void ReleaseGraphicsResources(vtkWindow *) override;

  int SetTextureAndRenderers(const char *filename, vtkRenderer *bg, vtkRenderer *canvas);

  void SetSphereMode(bool);
  void SetImageRepetitions(int,int);
protected:
   vtkTexturedBackgroundActor();
  ~vtkTexturedBackgroundActor();

   vtkMapper2D *GetInitializedMapper();
   vtkTexture  *GetTexture(const char *);
   void         SetTextureAndRenderers(vtkTexture *tex, vtkRenderer *bg, vtkRenderer *canvas);

   vtkTexture  *texture;
   char        *imageFile;
};

#endif
