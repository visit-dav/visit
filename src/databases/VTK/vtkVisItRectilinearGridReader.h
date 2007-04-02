/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItRectilinearGridReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
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
  vtkRectilinearGrid *GetOutput(int idx);
  void SetOutput(vtkRectilinearGrid *output);

  // Description:
  // Read the meta information from the file.  This needs to be public to it
  // can be accessed by vtkDataSetReader.
  virtual int ReadMetaData(vtkInformation *outInfo);

protected:
  vtkVisItRectilinearGridReader();
  ~vtkVisItRectilinearGridReader();

  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *);

  virtual int FillOutputPortInformation(int, vtkInformation*);
private:
  vtkVisItRectilinearGridReader(const vtkVisItRectilinearGridReader&);  // Not implemented.
  void operator=(const vtkVisItRectilinearGridReader&);  // Not implemented.
};

#endif


