/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLRectilinearGridReader.h,v $
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
// .NAME vtkVisItXMLRectilinearGridReader - Read VTK XML RectilinearGrid files.
// .SECTION Description
// vtkVisItXMLRectilinearGridReader reads the VTK XML RectilinearGrid file
// format.  One rectilinear grid file can be read to produce one
// output.  Streaming is supported.  The standard extension for this
// reader's file format is "vtr".  This reader is also used to read a
// single piece of the parallel file format.

// .SECTION See Also
// vtkVisItXMLPRectilinearGridReader

#ifndef __vtkVisItXMLRectilinearGridReader_h
#define __vtkVisItXMLRectilinearGridReader_h

#include "vtkVisItXMLStructuredDataReader.h"

class vtkRectilinearGrid;

class vtkVisItXMLRectilinearGridReader : public vtkVisItXMLStructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLRectilinearGridReader,vtkVisItXMLStructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkVisItXMLRectilinearGridReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkRectilinearGrid *output);
  vtkRectilinearGrid *GetOutput();
  vtkRectilinearGrid *GetOutput(int idx);
  
protected:
  vtkVisItXMLRectilinearGridReader();
  ~vtkVisItXMLRectilinearGridReader();  
  
  const char* GetDataSetName();
  void SetOutputExtent(int* extent);
  
  void SetupPieces(int numPieces);
  void DestroyPieces();
  void SetupOutputInformation();
  void SetupOutputData();
  int ReadPiece(vtkVisItXMLDataElement* ePiece);
  int ReadPieceData();
  int ReadSubCoordinates(int* inBounds, int* outBounds, int* subBounds,
                         vtkVisItXMLDataElement* da, vtkDataArray* array);
  
  // The elements representing the coordinate arrays for each piece.
  vtkVisItXMLDataElement** CoordinateElements;
  
private:
  vtkVisItXMLRectilinearGridReader(const vtkVisItXMLRectilinearGridReader&);  // Not implemented.
  void operator=(const vtkVisItXMLRectilinearGridReader&);  // Not implemented.
};

#endif
