/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItPolyDataReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItPolyDataReader - read vtk polygonal data file
// .SECTION Description
// vtkVisItPolyDataReader is a source object that reads ASCII or binary 
// polygonal data files in vtk format (see text for format details).
// The output of this reader is a single vtkPolyData data object.
// The superclass of this class, vtkVisItDataReader, provides many methods for
// controlling the reading of the data file, see vtkVisItDataReader for more
// information.
// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// .SECTION See Also
// vtkPolyData vtkVisItDataReader

#ifndef __vtkVisItPolyDataReader_h
#define __vtkVisItPolyDataReader_h

#include "vtkVisItDataReader.h"

class vtkPolyData;

class vtkVisItPolyDataReader : public vtkVisItDataReader
{
public:
  static vtkVisItPolyDataReader *New();
  vtkTypeRevisionMacro(vtkVisItPolyDataReader,vtkVisItDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output of this reader.
  vtkPolyData *GetOutput();
  vtkPolyData *GetOutput(int idx);
  void SetOutput(vtkPolyData *output);

protected:
  vtkVisItPolyDataReader();
  ~vtkVisItPolyDataReader();

  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *);

  // Update extent of PolyData is specified in pieces.  
  // Since all DataObjects should be able to set UpdateExent as pieces,
  // just copy output->UpdateExtent  all Inputs.
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
                                  vtkInformationVector *);
  
  virtual int FillOutputPortInformation(int, vtkInformation*);

  // Used by streaming: The extent of the output being processed
  // by the execute method. Set in the ComputeInputUpdateExtents method.
  int ExecutePiece;
  int ExecuteNumberOfPieces;
  int ExecuteGhostLevel;
private:
  vtkVisItPolyDataReader(const vtkVisItPolyDataReader&);  // Not implemented.
  void operator=(const vtkVisItPolyDataReader&);  // Not implemented.
};

#endif


