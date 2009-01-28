/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOpenGLTexture.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItOpenGLTexture - OpenGL texture map
// .SECTION Description
// vtkVisItOpenGLTexture is a concrete implementation of the abstract class 
// vtkTexture. vtkVisItOpenGLTexture interfaces to the OpenGL rendering library.

#ifndef __vtkVisItOpenGLTexture_h
#define __vtkVisItOpenGLTexture_h

#include "vtkTexture.h"

class vtkWindow;
class vtkOpenGLRenderer;
class vtkRenderWindow;

// Not exposed in DLL on purpose
class vtkVisItOpenGLTexture : public vtkTexture
{
public:
  static vtkVisItOpenGLTexture *New();
  vtkTypeRevisionMacro(vtkVisItOpenGLTexture,vtkTexture);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Load(vtkRenderer *ren);
  
  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release. Using the same texture object in multiple
  // render windows is NOT currently supported. 
  void ReleaseGraphicsResources(vtkWindow *);

protected:
  vtkVisItOpenGLTexture();
  ~vtkVisItOpenGLTexture();

  unsigned char *ResampleToPowerOfTwo(int &xsize, int &ysize, 
                                      unsigned char *dptr, int bpp);

  vtkTimeStamp   LoadTime;
  long          Index;
  vtkRenderWindow *RenderWindow;   // RenderWindow used for previous render
private:
  vtkVisItOpenGLTexture(const vtkVisItOpenGLTexture&);  // Not implemented.
  void operator=(const vtkVisItOpenGLTexture&);  // Not implemented.
};

#endif
