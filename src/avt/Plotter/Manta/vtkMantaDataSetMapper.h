/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDataSetMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDataSetMapper - map vtkDataSet and derived classes to graphics primitives
// .SECTION Description
// vtkDataSetMapper is a mapper to map data sets (i.e., vtkDataSet and 
// all derived classes) to graphics primitives. The mapping procedure
// is as follows: all 0D, 1D, and 2D cells are converted into points,
// lines, and polygons/triangle strips and then mapped to the graphics 
// system. The 2D faces of 3D cells are mapped only if they are used by 
// only one cell, i.e., on the boundary of the data set.

#ifndef __vtkMantaDataSetMapper_h
#define __vtkMantaDataSetMapper_h

#include <vtkVisItDataSetMapper.h>


class vtkPolyDataMapper;
class vtkDataSetSurfaceFilter;
//class vtkDataSetMapper;
class vtkMantaPolyDataMapper;

class vtkMantaDataSetMapper : public vtkVisItDataSetMapper
{
public:
  static vtkMantaDataSetMapper *New() { return new vtkMantaDataSetMapper(); }
  //  vtkTypeMacro(vtkDataSetMapper,vtkMapper) {}
  //  vtkTypeMacro(vtkDataSetMapper,vtkMapper);

  // Description:
  // Get the internal poly data mapper used to map data set to graphics system.
  //  vtkGetObjectMacro(PolyDataMapper, vtkPolyDataMapper);

  void Render(vtkRenderer *ren, vtkActor *act);
protected:
 vtkMantaDataSetMapper();
 virtual ~vtkMantaDataSetMapper();
 vtkMantaPolyDataMapper* mapper;

private:
  //vtkDataSetMapper(const vtkDataSetMapper&);  // Not implemented.
  //void operator=(const vtkDataSetMapper&);  // Not implemented.
};

#endif


