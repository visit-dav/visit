/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItRectilinearGridReader.h,v $
  Language:  C++
  Date:      $Date: 2002/05/31 23:12:41 $
  Version:   $Revision: 1.31 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItRectilinearGridReader - read vtk rectilinear grid data file
// .SECTION Description
// vtkVisItRectilinearGridReader is a source object that reads ASCII or binary 
// rectilinear grid data files in vtk format (see text for format details).
// The output of this reader is a single vtkRectilinearGrid data object.
// The superclass of this class, vtkVisItDataReader, provides many methods for
// controlling the reading of the data file, see vtkVisItDataReader for more
// information.
// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// .SECTION See Also
// vtkRectilinearGrid vtkVisItDataReader

#ifndef __vtkVisItRectilinearGridReader_h
#define __vtkVisItRectilinearGridReader_h

#include "vtkVisItDataReader.h"

class vtkRectilinearGrid;

class vtkVisItRectilinearGridReader : public vtkVisItDataReader
{
public:
  static vtkVisItRectilinearGridReader *New();
  vtkTypeRevisionMacro(vtkVisItRectilinearGridReader,vtkVisItDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get and set the output of this reader.
  vtkRectilinearGrid *GetOutput();
  vtkRectilinearGrid *GetOutput(int idx)
    {return (vtkRectilinearGrid *) this->vtkSource::GetOutput(idx); };
  void SetOutput(vtkRectilinearGrid *output);

protected:
  vtkVisItRectilinearGridReader();
  ~vtkVisItRectilinearGridReader();

  void Execute();
  void ExecuteInformation();

private:
  vtkVisItRectilinearGridReader(const vtkVisItRectilinearGridReader&);  // Not implemented.
  void operator=(const vtkVisItRectilinearGridReader&);  // Not implemented.
};

#endif


