/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItStructuredGridReader.h,v $
  Language:  C++
  Date:      $Date: 2002/05/31 23:12:41 $
  Version:   $Revision: 1.46 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItStructuredGridReader - read vtk structured grid data file
// .SECTION Description
// vtkVisItStructuredGridReader is a source object that reads ASCII or binary 
// structured grid data files in vtk format. (see text for format details).
// The output of this reader is a single vtkStructuredGrid data object.
// The superclass of this class, vtkVisItDataReader, provides many methods for
// controlling the reading of the data file, see vtkVisItDataReader for more
// information.
// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// .SECTION See Also
// vtkStructuredGrid vtkVisItDataReader

#ifndef __vtkVisItStructuredGridReader_h
#define __vtkVisItStructuredGridReader_h

#include "vtkVisItDataReader.h"

class vtkStructuredGrid;

class vtkVisItStructuredGridReader : public vtkVisItDataReader
{
public:
  static vtkVisItStructuredGridReader *New();
  vtkTypeRevisionMacro(vtkVisItStructuredGridReader,vtkVisItDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output of this reader.
  vtkStructuredGrid *GetOutput();
  vtkStructuredGrid *GetOutput(int idx)
    {return (vtkStructuredGrid *) this->vtkSource::GetOutput(idx); };
  void SetOutput(vtkStructuredGrid *output);  

protected:
  vtkVisItStructuredGridReader();
  ~vtkVisItStructuredGridReader();

  void ExecuteInformation();
  void Execute();

private:
  vtkVisItStructuredGridReader(const vtkVisItStructuredGridReader&);  // Not implemented.
  void operator=(const vtkVisItStructuredGridReader&);  // Not implemented.
};

#endif


