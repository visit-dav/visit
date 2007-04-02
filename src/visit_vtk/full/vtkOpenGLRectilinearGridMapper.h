/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOpenGLRectilinearGridMapper.h,v $
  Language:  C++
  Date:      $Date: 2002/08/22 18:39:31 $
  Version:   $Revision: 1.31 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOpenGLRectilinearGridMapper - a RectilinearGridMapper for the OpenGL library
// .SECTION Description
// vtkOpenGLRectilinearGridMapper is a subclass of vtkRectilinearGridMapper.
// vtkOpenGLRectilinearGridMapper is a geometric RectilinearGridMapper for the OpenGL 
// rendering library.

#ifndef __vtkOpenGLRectilinearGridMapper_h
#define __vtkOpenGLRectilinearGridMapper_h

#include "vtkRectilinearGridMapper.h"
#include <visit_vtk_exports.h>

class vtkProperty;
class vtkRenderWindow;
class vtkOpenGLRenderer;

class VISIT_VTK_API vtkOpenGLRectilinearGridMapper : public vtkRectilinearGridMapper
{
public:
  static vtkOpenGLRectilinearGridMapper *New();
  vtkTypeRevisionMacro(vtkOpenGLRectilinearGridMapper,vtkRectilinearGridMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement superclass render method.
  virtual void Render(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Sets/Gets the whether color texturing is enabled.
  vtkSetMacro(EnableColorTexturing, bool);
  vtkGetMacro(EnableColorTexturing, bool);

protected:
  vtkOpenGLRectilinearGridMapper();
  ~vtkOpenGLRectilinearGridMapper();

  int ListStart;
  int CurrentList;
  int nLists;
  bool doingDisplayLists;
  int  primsInCurrentList;

  bool          EnableColorTexturing;
  bool          ColorTexturingAllowed;
  bool          ColorTextureLoaded;
  bool          ColorTextureLooksDiscrete;
  unsigned int  ColorTextureName;
  float        *ColorTexture;
  int           ColorTextureSize;
  bool          OpenGLSupportsVersion1_2;
  bool          GLEW_initialized;

  bool MapScalarsWithTextureSupport(double);
  void BeginColorTexturing();
  void EndColorTexturing();
  bool UsesPointData(vtkDataSet *input, int scalarMode,
                     int arrayAccessMode, int arrayId, const char *arrayName,
                     int& offset);

private:
  vtkOpenGLRectilinearGridMapper(const vtkOpenGLRectilinearGridMapper&);  // Not implemented.
  void operator=(const vtkOpenGLRectilinearGridMapper&);  // Not implemented.
};

#endif
