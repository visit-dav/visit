/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItMesaPolyDataMapper.h,v $
  Language:  C++
  Date:      $Date: 2002/08/22 18:39:30 $
  Version:   $Revision: 1.20 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItMesaPolyDataMapper - a PolyDataMapper for the Mesa library
// .SECTION Description
// vtkVisItMesaPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkVisItMesaPolyDataMapper is a geometric PolyDataMapper for the Mesa 
// rendering library.

#ifndef __vtkVisItMesaPolyDataMapper_h
#define __vtkVisItMesaPolyDataMapper_h

#include "vtkPolyDataMapper.h"

class vtkProperty;
class vtkRenderWindow;
class vtkMesaRenderer;
class vtkTimerLog;

class VTK_RENDERING_EXPORT vtkVisItMesaPolyDataMapper : public vtkPolyDataMapper
{
public:
  static vtkVisItMesaPolyDataMapper *New();
  vtkTypeRevisionMacro(vtkVisItMesaPolyDataMapper,vtkPolyDataMapper);
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
  // Draw method for Mesa.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);
  
protected:
  vtkVisItMesaPolyDataMapper();
  ~vtkVisItMesaPolyDataMapper();

  int ListId;
  vtkRenderWindow *RenderWindow;   // RenderWindow used for the previous render
private:
  vtkVisItMesaPolyDataMapper(const vtkVisItMesaPolyDataMapper&);  // Not implemented.
  void operator=(const vtkVisItMesaPolyDataMapper&);  // Not implemented.
};

#endif
