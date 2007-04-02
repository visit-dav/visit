/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLUnstructuredGridReader.h,v $
  Language:  C++
  Date:      $Date: 2003/05/05 13:45:23 $
  Version:   $Revision: 1.3 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLUnstructuredGridReader - Read VTK XML UnstructuredGrid files.
// .SECTION Description
// vtkVisItXMLUnstructuredGridReader reads the VTK XML UnstructuredGrid
// file format.  One unstructured grid file can be read to produce one
// output.  Streaming is supported.  The standard extension for this
// reader's file format is "vtu".  This reader is also used to read a
// single piece of the parallel file format.

// .SECTION See Also
// vtkVisItXMLPUnstructuredGridReader

#ifndef __vtkVisItXMLUnstructuredGridReader_h
#define __vtkVisItXMLUnstructuredGridReader_h

#include "vtkVisItXMLUnstructuredDataReader.h"

class vtkUnstructuredGrid;

class vtkVisItXMLUnstructuredGridReader : public vtkVisItXMLUnstructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLUnstructuredGridReader,vtkVisItXMLUnstructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);  
  static vtkVisItXMLUnstructuredGridReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkUnstructuredGrid *output);
  vtkUnstructuredGrid *GetOutput();
  vtkUnstructuredGrid *GetOutput(int idx);
  
protected:
  vtkVisItXMLUnstructuredGridReader();
  ~vtkVisItXMLUnstructuredGridReader();
  
  const char* GetDataSetName();
  void GetOutputUpdateExtent(int& piece, int& numberOfPieces, int& ghostLevel);
  void SetupOutputTotals();
  void SetupPieces(int numPieces);
  void DestroyPieces();
  
  void SetupOutputData();
  int ReadPiece(vtkVisItXMLDataElement* ePiece);
  void SetupNextPiece();
  int ReadPieceData();
  
  // Read a data array whose tuples coorrespond to cells.
  int ReadArrayForCells(vtkVisItXMLDataElement* da, vtkDataArray* outArray);
  
  // Get the number of cells in the given piece.  Valid after
  // UpdateInformation.
  virtual vtkIdType GetNumberOfCellsInPiece(int piece);

  // The index of the cell in the output where the current piece
  // begins.
  vtkIdType StartCell;
  
  // The Cells element for each piece.
  vtkVisItXMLDataElement** CellElements;
  vtkIdType* NumberOfCells;
  
private:
  vtkVisItXMLUnstructuredGridReader(const vtkVisItXMLUnstructuredGridReader&);  // Not implemented.
  void operator=(const vtkVisItXMLUnstructuredGridReader&);  // Not implemented.
};

#endif
