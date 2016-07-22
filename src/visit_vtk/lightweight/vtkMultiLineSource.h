/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkLineSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLineSource - create a line defined by two end points
// .SECTION Description
// vtkLineSource is a source object that creates a polyline defined by
// two endpoints. The number of segments composing the polyline is
// controlled by setting the object resolution.
//
// .SECTION Thanks
// This class was extended by Philippe Pebay, Kitware SAS 2011, to support
// broken lines as well as simple lines.

// Modifications:
//   Matt Larsen, Fri Jul 7 07:30:37 PST 2016
//   Changed lineSource to a multiLineSouce to support pick highlights
//   Added AddLine to add lines
//   Added shift method to move lines from original points by a supplied vector
//
//   Matt Larsen, Thus Jul 21 14:00:12 PST 2016
//   Corrected VTK_LIGHT_API macro

#ifndef vtkMultiLineSource_h
#define vtkMultiLineSource_h

#include "vtkPolyDataAlgorithm.h"
#include <visit_vtk_light_exports.h>
class vtkPoints;

class VISIT_VTK_LIGHT_API vtkMultiLineSource : public vtkPolyDataAlgorithm
{
public:
  static vtkMultiLineSource *New();
  vtkTypeMacro(vtkMultiLineSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddLine(double[3],double[3]);
  void Shift3d(const double[3], const double &);
  void Shift2d(const double &);

  // Description:
  // Set/Get the list of points defining a broken line
  virtual void SetPoints(vtkPoints*);
  vtkGetObjectMacro(Points,vtkPoints);

protected:
  vtkMultiLineSource();
  virtual ~vtkMultiLineSource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  //All the points of a the lines i*2+0 and i*2+1
  //We are supporting a shift from the original position
  //so we need to keep both around.
  vtkPoints* Points;
  vtkPoints* OriginalPoints;

private:
  vtkMultiLineSource(const vtkMultiLineSource&);  // Not implemented.
  void operator=(const vtkMultiLineSource&);  // Not implemented.
};

#endif
