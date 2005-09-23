/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLUnstructuredDataReader.h,v $
  Language:  C++
  Date:      $Date: 2003/05/05 13:45:23 $
  Version:   $Revision: 1.4 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLUnstructuredDataReader - Superclass for unstructured data XML readers.
// .SECTION Description
// vtkVisItXMLUnstructuredDataReader provides functionality common to all
// unstructured data format readers.

// .SECTION See Also
// vtkVisItXMLPolyDataReader vtkVisItXMLUnstructuredGridReader

#ifndef __vtkVisItXMLUnstructuredDataReader_h
#define __vtkVisItXMLUnstructuredDataReader_h

#include "vtkVisItXMLDataReader.h"

class vtkCellArray;
class vtkIdTypeArray;
class vtkPointSet;
class vtkUnsignedCharArray;

class vtkVisItXMLUnstructuredDataReader : public vtkVisItXMLDataReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLUnstructuredDataReader,vtkVisItXMLDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);  
  
  // Description:
  // Get the number of points in the output.
  virtual vtkIdType GetNumberOfPoints();
  
  // Description:
  // Get the number of cells in the output.
  virtual vtkIdType GetNumberOfCells();
  
  // Description:
  // Setup the reader as if the given update extent were requested by
  // its output.  This can be used after an UpdateInformation to
  // validate GetNumberOfPoints() and GetNumberOfCells() without
  // actually reading data.
  void SetupUpdateExtent(int piece, int numberOfPieces, int ghostLevel);
  
protected:
  vtkVisItXMLUnstructuredDataReader();
  ~vtkVisItXMLUnstructuredDataReader();
  
  vtkPointSet* GetOutputAsPointSet();
  vtkVisItXMLDataElement* FindDataArrayWithName(vtkVisItXMLDataElement* eParent,
                                           const char* name);
  vtkIdTypeArray* ConvertToIdTypeArray(vtkDataArray* a);
  vtkUnsignedCharArray* ConvertToUnsignedCharArray(vtkDataArray* a);
  
  // Pipeline execute data driver.  Called by vtkVisItXMLReader.
  void ReadXMLData();
  
  virtual void SetupEmptyOutput();
  virtual void GetOutputUpdateExtent(int& piece, int& numberOfPieces,
                                     int& ghostLevel)=0;
  virtual void SetupOutputTotals();
  virtual void SetupNextPiece();
  void SetupPieces(int numPieces);
  void DestroyPieces();
  
  void SetupOutputInformation();
  void SetupOutputData();
  int ReadPiece(vtkVisItXMLDataElement* ePiece);
  int ReadPieceData();
  int ReadCellArray(vtkIdType numberOfCells, vtkIdType totalNumberOfCells,
                    vtkVisItXMLDataElement* eCells, vtkCellArray* outCells);
  
  // Read a data array whose tuples coorrespond to points.
  int ReadArrayForPoints(vtkVisItXMLDataElement* da, vtkDataArray* outArray);
  
  // Get the number of points/cells in the given piece.  Valid after
  // UpdateInformation.
  virtual vtkIdType GetNumberOfPointsInPiece(int piece);
  virtual vtkIdType GetNumberOfCellsInPiece(int piece)=0;
  
  // The update request.
  int UpdatePiece;
  int UpdateNumberOfPieces;
  int UpdateGhostLevel;
  
  // The range of pieces from the file that will form the UpdatePiece.
  int StartPiece;
  int EndPiece;
  vtkIdType TotalNumberOfPoints;
  vtkIdType TotalNumberOfCells;
  vtkIdType StartPoint;
  
  // The Points element for each piece.
  vtkVisItXMLDataElement** PointElements;
  vtkIdType* NumberOfPoints;
  
private:
  vtkVisItXMLUnstructuredDataReader(const vtkVisItXMLUnstructuredDataReader&);  // Not implemented.
  void operator=(const vtkVisItXMLUnstructuredDataReader&);  // Not implemented.
};

#endif
