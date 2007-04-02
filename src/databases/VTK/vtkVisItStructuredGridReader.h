/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItStructuredGridReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
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
  vtkStructuredGrid *GetOutput(int idx);
  void SetOutput(vtkStructuredGrid *output);  

  // Description:
  // Read the meta information from the file.  This needs to be public to it
  // can be accessed by vtkDataSetReader.
  virtual int ReadMetaData(vtkInformation *outInfo);

protected:
  vtkVisItStructuredGridReader();
  ~vtkVisItStructuredGridReader();

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *);
  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *);

  virtual int FillOutputPortInformation(int, vtkInformation*);
private:
  vtkVisItStructuredGridReader(const vtkVisItStructuredGridReader&);  // Not implemented.
  void operator=(const vtkVisItStructuredGridReader&);  // Not implemented.
};

#endif


