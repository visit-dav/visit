/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItDataSetMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItDataSetMapper - map vtkDataSet and derived classes to 
// graphics primitives
// .SECTION Description
// vtkVisItDataSetMapper is a mapper to map data sets (i.e., vtkDataSet and 
// all derived classes) to graphics primitives. The mapping procedure
// is as follows: all 0D, 1D, and 2D cells are converted into points,
// lines, and polygons/triangle strips and then mapped to the graphics 
// system. The 2D faces of 3D cells are mapped only if they are used by 
// only one cell, i.e., on the boundary of the data set.

#ifndef __vtkVisItDataSetMapper_h
#define __vtkVisItDataSetMapper_h
#include <visit_vtk_exports.h>
#include "vtkMapper.h"

class vtkPolyDataMapper;
class vtkDataSetSurfaceFilter;

class VISIT_VTK_API vtkVisItDataSetMapper : public vtkMapper 
{
public:
  static vtkVisItDataSetMapper *New();
  vtkTypeRevisionMacro(vtkVisItDataSetMapper,vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent);
  void Render(vtkRenderer *ren, vtkActor *act);

  // Description:
  // Get the internal poly data mapper used to map data set to graphics system.
  vtkGetObjectMacro(PolyDataMapper, vtkPolyDataMapper);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Get the mtime also considering the lookup table.
  unsigned long GetMTime();

  // Description:
  // Set the Input of this mapper.
  void SetInput(vtkDataSet *input);
  vtkDataSet *GetInput();

  typedef enum {TEXTURE_NO_POINTS,
                TEXTURE_USING_POINTSPRITES
                // room for more methods such as shaders
               } PointTextureMode;

  // Description:
  // Sets/Gets the point texturing method. 
  void SetPointTextureMethod(PointTextureMode);
  vtkGetMacro(PointTextureMethod, PointTextureMode);

  // Description:
  // Sets/Gets the color texturing mode. 
  void SetEnableColorTexturing(bool);
  vtkGetMacro(EnableColorTexturing, bool);

protected:
  vtkVisItDataSetMapper();
  ~vtkVisItDataSetMapper();

  vtkDataSetSurfaceFilter *GeometryExtractor;
  vtkPolyDataMapper       *PolyDataMapper;
  PointTextureMode         PointTextureMethod;
  bool                     EnableColorTexturing;

  virtual void ReportReferences(vtkGarbageCollector*);
  virtual int FillInputPortInformation(int port, vtkInformation *info);
private:
  vtkVisItDataSetMapper(const vtkVisItDataSetMapper&);  // Not implemented.
  void operator=(const vtkVisItDataSetMapper&);  // Not implemented.
};

#endif


