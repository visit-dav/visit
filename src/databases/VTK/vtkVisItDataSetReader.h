/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItDataSetReader.h,v $
  Language:  C++
  Date:      $Date: 2002/05/31 23:12:41 $
  Version:   $Revision: 1.54 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItDataSetReader - class to read any type of vtk dataset
// .SECTION Description
// vtkVisItDataSetReader is a class that provides instance variables and methods
// to read any type of dataset in Visualization Toolkit (vtk) format.  The
// output type of this class will vary depending upon the type of data
// file. Convenience methods are provided to keep the data as a particular
// type. (See text for format description details).
// The superclass of this class, vtkVisItDataReader, provides many methods for
// controlling the reading of the data file, see vtkVisItDataReader for more
// information.
// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// .SECTION See Also
// vtkVisItDataReader vtkVisItPolyDataReader vtkVisItRectilinearGridReader 
// vtkVisItStructuredPointsReader vtkVisItStructuredGridReader vtkVisItUnstructuredGridReader

#ifndef __vtkVisItDataSetReader_h
#define __vtkVisItDataSetReader_h

#include "vtkVisItDataReader.h"

class vtkDataSet;
class vtkPolyData;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkStructuredPoints;
class vtkUnstructuredGrid;

class vtkVisItDataSetReader : public vtkVisItDataReader
{
public:
  static vtkVisItDataSetReader *New();
  vtkTypeRevisionMacro(vtkVisItDataSetReader,vtkVisItDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output of this source as a general vtkDataSet. Since we need 
  // to know the type of the data, the FileName must be set before GetOutput 
  // is applied.
  vtkDataSet *GetOutput();
  vtkDataSet *GetOutput(int idx);

  // Description:
  // Get the output as various concrete types. This method is typically used
  // when you know exactly what type of data is being read.  Otherwise, use
  // the general GetOutput() method. If the wrong type is used NULL is
  // returned.  (You must also set the filename of the object prior to
  // getting the output.)
  vtkPolyData *GetPolyDataOutput();
  vtkStructuredPoints *GetStructuredPointsOutput();
  vtkStructuredGrid *GetStructuredGridOutput();
  vtkUnstructuredGrid *GetUnstructuredGridOutput();
  vtkRectilinearGrid *GetRectilinearGridOutput();

  // Description:
  // If there is no output, execute anyway.  Execute creates an output.
  void Update();
  
  // Description:
  // This method can be used to find out the type of output expected without
  // needing to read the whole file.
  virtual int ReadOutputType();

protected:
  vtkVisItDataSetReader();
  ~vtkVisItDataSetReader();

  void Execute();
  vtkVisItDataReader *Reader;

private:
  vtkVisItDataSetReader(const vtkVisItDataSetReader&);  // Not implemented.
  void operator=(const vtkVisItDataSetReader&);  // Not implemented.
};

#endif


