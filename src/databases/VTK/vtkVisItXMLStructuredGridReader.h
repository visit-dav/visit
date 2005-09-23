/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLStructuredGridReader.h,v $
  Language:  C++
  Date:      $Date: 2003/02/07 20:06:29 $
  Version:   $Revision: 1.2 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLStructuredGridReader - Read VTK XML StructuredGrid files.
// .SECTION Description
// vtkVisItXMLStructuredGridReader reads the VTK XML StructuredGrid file
// format.  One structured grid file can be read to produce one
// output.  Streaming is supported.  The standard extension for this
// reader's file format is "vts".  This reader is also used to read a
// single piece of the parallel file format.

// .SECTION See Also
// vtkVisItXMLPStructuredGridReader

#ifndef __vtkVisItXMLStructuredGridReader_h
#define __vtkVisItXMLStructuredGridReader_h

#include "vtkVisItXMLStructuredDataReader.h"

class vtkStructuredGrid;

class vtkVisItXMLStructuredGridReader : public vtkVisItXMLStructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLStructuredGridReader,vtkVisItXMLStructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkVisItXMLStructuredGridReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkStructuredGrid *output);
  vtkStructuredGrid *GetOutput();
  vtkStructuredGrid *GetOutput(int idx);

protected:
  vtkVisItXMLStructuredGridReader();
  ~vtkVisItXMLStructuredGridReader();  
  
  const char* GetDataSetName();
  void SetOutputExtent(int* extent);
  
  void SetupPieces(int numPieces);
  void DestroyPieces();
  void SetupOutputInformation();
  void SetupOutputData();
  
  int ReadPiece(vtkVisItXMLDataElement* ePiece);
  int ReadPieceData();
  
  // The elements representing the points for each piece.
  vtkVisItXMLDataElement** PointElements;
  
private:
  vtkVisItXMLStructuredGridReader(const vtkVisItXMLStructuredGridReader&);  // Not implemented.
  void operator=(const vtkVisItXMLStructuredGridReader&);  // Not implemented.
};

#endif
