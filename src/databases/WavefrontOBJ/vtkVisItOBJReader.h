/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOBJReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItOBJReader - read Wavefront .obj files
// .SECTION Description
// vtkVisItOBJReader is a source object that reads Wavefront .obj
// files. The output of this source object is polygonal data.
// .SECTION See Also
// vtkVisItOBJImporter

#ifndef __vtkVisItOBJReader_h
#define __vtkVisItOBJReader_h

#include "vtkPolyDataAlgorithm.h"

class vtkVisItOBJReader : public vtkPolyDataAlgorithm 
{
public:
  static vtkVisItOBJReader *New();
  vtkTypeRevisionMacro(vtkVisItOBJReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of Wavefront .obj file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkVisItOBJReader();
  ~vtkVisItOBJReader();
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  char *FileName;
private:
  vtkVisItOBJReader(const vtkVisItOBJReader&);  // Not implemented.
  void operator=(const vtkVisItOBJReader&);  // Not implemented.
};

#endif
