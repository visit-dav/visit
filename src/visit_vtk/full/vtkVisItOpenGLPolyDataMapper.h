/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOpenGLPolyDataMapper.h,v $
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
// .NAME vtkVisItOpenGLPolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkVisItOpenGLPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkVisItOpenGLPolyDataMapper is a geometric PolyDataMapper for the OpenGL 
// rendering library.

#ifndef __vtkVisItOpenGLPolyDataMapper_h
#define __vtkVisItOpenGLPolyDataMapper_h

#include "vtkPolyDataMapper.h"
#include <visit_vtk_exports.h>

class vtkProperty;
class vtkRenderWindow;
class vtkOpenGLRenderer;

// ****************************************************************************
//  Modifications:
//  
//    Hank Childs, Tue May 25 10:04:36 PDT 2004
//    Break display lists up into smaller display lists.
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItOpenGLPolyDataMapper : public vtkPolyDataMapper
{
public:
  static vtkVisItOpenGLPolyDataMapper *New();
  vtkTypeRevisionMacro(vtkVisItOpenGLPolyDataMapper,vtkPolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement superclass render method.
  virtual void RenderPiece(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);
  
protected:
  vtkVisItOpenGLPolyDataMapper();
  ~vtkVisItOpenGLPolyDataMapper();

  int ListStart;
  int CurrentList;
  int nLists;
  bool doingDisplayLists;
  int  primsInCurrentList;

private:
  vtkVisItOpenGLPolyDataMapper(const vtkVisItOpenGLPolyDataMapper&);  // Not implemented.
  void operator=(const vtkVisItOpenGLPolyDataMapper&);  // Not implemented.
};

#endif
