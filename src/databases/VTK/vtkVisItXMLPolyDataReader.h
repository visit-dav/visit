/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLPolyDataReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLPolyDataReader - Read VTK XML PolyData files.
// .SECTION Description
// vtkVisItXMLPolyDataReader reads the VTK XML PolyData file format.  One
// polygonal data file can be read to produce one output.  Streaming
// is supported.  The standard extension for this reader's file format
// is "vtp".  This reader is also used to read a single piece of the
// parallel file format.

// .SECTION See Also
// vtkVisItXMLPPolyDataReader

#ifndef __vtkVisItXMLPolyDataReader_h
#define __vtkVisItXMLPolyDataReader_h

#include "vtkVisItXMLUnstructuredDataReader.h"

class vtkPolyData;

class vtkVisItXMLPolyDataReader : public vtkVisItXMLUnstructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLPolyDataReader,vtkVisItXMLUnstructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);  
  static vtkVisItXMLPolyDataReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkPolyData *output);
  vtkPolyData *GetOutput();
  vtkPolyData *GetOutput(int idx);
  
  // Description:
  // Get the number of verts/lines/strips/polys in the output.
  virtual vtkIdType GetNumberOfVerts();
  virtual vtkIdType GetNumberOfLines();
  virtual vtkIdType GetNumberOfStrips();
  virtual vtkIdType GetNumberOfPolys();
  
protected:
  vtkVisItXMLPolyDataReader();
  ~vtkVisItXMLPolyDataReader();
  
  const char* GetDataSetName();
  void GetOutputUpdateExtent(int& piece, int& numberOfPieces, int& ghostLevel);
  void SetupOutputTotals();
  void SetupNextPiece();
  void SetupPieces(int numPieces);
  void DestroyPieces();
  
  void SetupOutputData();
  int ReadPiece(vtkVisItXMLDataElement* ePiece);
  int ReadPieceData();
  
  // Read a data array whose tuples coorrespond to cells.
  int ReadArrayForCells(vtkVisItXMLDataElement* da, vtkDataArray* outArray);
  
  // Get the number of cells in the given piece.  Valid after
  // UpdateInformation.
  virtual vtkIdType GetNumberOfCellsInPiece(int piece);

  virtual int FillOutputPortInformation(int, vtkInformation*);

  // The size of the UpdatePiece.
  int TotalNumberOfVerts;
  int TotalNumberOfLines;
  int TotalNumberOfStrips;
  int TotalNumberOfPolys;
  vtkIdType StartVert;
  vtkIdType StartLine;
  vtkIdType StartStrip;
  vtkIdType StartPoly;
  
  // The cell elements for each piece.
  vtkVisItXMLDataElement** VertElements;
  vtkVisItXMLDataElement** LineElements;
  vtkVisItXMLDataElement** StripElements;
  vtkVisItXMLDataElement** PolyElements;
  vtkIdType* NumberOfVerts;
  vtkIdType* NumberOfLines;
  vtkIdType* NumberOfStrips;
  vtkIdType* NumberOfPolys;

  // For TimeStep support
  int VertsTimeStep;
  unsigned long VertsOffset;
  int LinesTimeStep;
  unsigned long LinesOffset;
  int StripsTimeStep;
  unsigned long StripsOffset;
  int PolysTimeStep;
  unsigned long PolysOffset;

private:
  vtkVisItXMLPolyDataReader(const vtkVisItXMLPolyDataReader&);  // Not implemented.
  void operator=(const vtkVisItXMLPolyDataReader&);  // Not implemented.
};

#endif
