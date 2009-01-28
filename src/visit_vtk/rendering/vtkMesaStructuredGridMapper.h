/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMesaStructuredGridMapper.h,v $
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
// .NAME vtkMesaStructuredGridMapper - a StructuredGridMapper for the Mesa library
// .SECTION Description
// vtkMesaStructuredGridMapper is a subclass of vtkStructuredGridMapper.
// vtkMesaStructuredGridMapper is a geometric StructuredGridMapper for the Mesa 
// rendering library.

#ifndef __vtkMesaStructuredGridMapper_h
#define __vtkMesaStructuredGridMapper_h

#include "vtkStructuredGridMapper.h"
#include <rendering_visit_vtk_exports.h>

class vtkProperty;
class vtkRenderWindow;
class vtkMesaRenderer;
class vtkTimerLog;

class RENDERING_VISIT_VTK_API vtkMesaStructuredGridMapper : public vtkStructuredGridMapper
{
public:
  static vtkMesaStructuredGridMapper *New();
  vtkTypeRevisionMacro(vtkMesaStructuredGridMapper,vtkStructuredGridMapper);
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
  // Draw method for Mesa.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Sets/Gets the whether color texturing is enabled.
  vtkSetMacro(EnableColorTexturing, bool);
  vtkGetMacro(EnableColorTexturing, bool);

protected:
  vtkMesaStructuredGridMapper();
  ~vtkMesaStructuredGridMapper();

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
  double        LastOpacity;

  bool MapScalarsWithTextureSupport(double);
  void BeginColorTexturing();
  void EndColorTexturing();
  bool UsesPointData(vtkDataSet *input, int scalarMode,
                     int arrayAccessMode, int arrayId, const char *arrayName,
                     int& offset);
private:
  vtkMesaStructuredGridMapper(const vtkMesaStructuredGridMapper&);  // Not implemented.
  void operator=(const vtkMesaStructuredGridMapper&);  // Not implemented.
};

#endif
