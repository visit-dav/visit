/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkRectilinearGridMapper.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRectilinearGridMapper - map vtkRectilinearGrid to graphics primitives
// .SECTION Description
// vtkRectilinearGridMapper is a class that maps rectilinear data (i.e., vtkRectilinearGrid)
// to graphics primitives. vtkRectilinearGridMapper serves as a superclass for
// device-specific rectilinear data mappers, that actually do the mapping to the
// rendering/graphics hardware/software.

#ifndef __vtkRectilinearGridMapper_h
#define __vtkRectilinearGridMapper_h

#include "vtkMapper.h"

class vtkRectilinearGrid;
class vtkRenderer;

class VTK_RENDERING_EXPORT vtkRectilinearGridMapper : public vtkMapper 
{
public:
  static vtkRectilinearGridMapper *New();
  vtkTypeRevisionMacro(vtkRectilinearGridMapper,vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the input data to map.
  void SetInput(vtkRectilinearGrid *in);
  vtkRectilinearGrid *GetInput();
  
  // Description:
  // Make a shallow copy of this mapper.
  void ShallowCopy(vtkAbstractMapper *m);

  // Description:
  // Return bounding box (array of six doubles) of data expressed as
  // (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual double *GetBounds();
  virtual void GetBounds(double bounds[6])
    {this->Superclass::GetBounds(bounds);};

protected:  
  vtkRectilinearGridMapper();
  ~vtkRectilinearGridMapper() {};

  virtual int FillInputPortInformation(int, vtkInformation*);

private:
  vtkRectilinearGridMapper(const vtkRectilinearGridMapper&);  // Not implemented.
  void operator=(const vtkRectilinearGridMapper&);  // Not implemented.
};

#endif
