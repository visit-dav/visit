/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItStructuredPointsReader.h,v $
  Language:  C++
  Date:      $Date: 2002/05/31 23:12:41 $
  Version:   $Revision: 1.47 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItStructuredPointsReader - read vtk structured points data file
// .SECTION Description
// vtkVisItStructuredPointsReader is a source object that reads ASCII or binary 
// structured points data files in vtk format (see text for format details).
// The output of this reader is a single vtkStructuredPoints data object.
// The superclass of this class, vtkVisItDataReader, provides many methods for
// controlling the reading of the data file, see vtkVisItDataReader for more
// information.
// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// .SECTION See Also
// vtkStructuredPoints vtkVisItDataReader

#ifndef __vtkVisItStructuredPointsReader_h
#define __vtkVisItStructuredPointsReader_h

#include "vtkVisItDataReader.h"

class vtkStructuredPoints;

class vtkVisItStructuredPointsReader : public vtkVisItDataReader
{
public:
  static vtkVisItStructuredPointsReader *New();
  vtkTypeRevisionMacro(vtkVisItStructuredPointsReader,vtkVisItDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the output of this reader.
  void SetOutput(vtkStructuredPoints *output);
  vtkStructuredPoints *GetOutput(int idx)
    {return (vtkStructuredPoints *) this->vtkSource::GetOutput(idx); };
  vtkStructuredPoints *GetOutput();
  
protected:
  vtkVisItStructuredPointsReader();
  ~vtkVisItStructuredPointsReader();

  void Execute();

  // Default method performs Update to get information.  Not all the old
  // structured points sources compute information
  void ExecuteInformation();

private:
  vtkVisItStructuredPointsReader(const vtkVisItStructuredPointsReader&);  // Not implemented.
  void operator=(const vtkVisItStructuredPointsReader&);  // Not implemented.
};

#endif


