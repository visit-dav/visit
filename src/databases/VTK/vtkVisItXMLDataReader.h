/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLDataReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLDataReader - Superclass for VTK XML file readers.
// .SECTION Description
// vtkVisItXMLDataReader provides functionality common to all VTK XML file
// readers.  Concrete subclasses call upon this functionality when
// needed.

// .SECTION See Also
// vtkVisItXMLPDataReader

#ifndef __vtkVisItXMLDataReader_h
#define __vtkVisItXMLDataReader_h

#include "vtkVisItXMLReader.h"

class vtkVisItXMLDataReader : public vtkVisItXMLReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLDataReader,vtkVisItXMLReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Get the number of points in the output.
  virtual vtkIdType GetNumberOfPoints()=0;
  
  // Description:
  // Get the number of cells in the output.
  virtual vtkIdType GetNumberOfCells()=0;
  
  // For the specified port, copy the information this reader sets up in
  // SetupOutputInformation to outInfo
  virtual void CopyOutputInformation(vtkInformation *outInfo, int port);

protected:
  vtkVisItXMLDataReader();
  ~vtkVisItXMLDataReader();  
  
  // Add functionality to methods from superclass.
  virtual void CreateXMLParser();
  virtual void DestroyXMLParser();
  virtual void SetupOutputInformation(vtkInformation *outInfo);

  int ReadPrimaryElement(vtkVisItXMLDataElement* ePrimary);
  void SetupOutputData();
  
  // Setup the reader for a given number of pieces.
  virtual void SetupPieces(int numPieces);
  virtual void DestroyPieces();
  
  // Read information from the file for the given piece.
  int ReadPiece(vtkVisItXMLDataElement* ePiece, int piece);
  virtual int ReadPiece(vtkVisItXMLDataElement* ePiece);
  
  // Read data from the file for the given piece.
  int ReadPieceData(int piece);
  virtual int ReadPieceData();
  
  virtual void ReadXMLData();

  // Read a data array whose tuples coorrespond to points or cells.
  virtual int ReadArrayForPoints(vtkVisItXMLDataElement* da,
                                 vtkDataArray* outArray);
  virtual int ReadArrayForCells(vtkVisItXMLDataElement* da,
                                vtkDataArray* outArray);
  
  // Read data from a given element.
  int ReadData(vtkVisItXMLDataElement* da, void* data, int wordType,
               vtkIdType startWord, vtkIdType numWords);
  
  // Callback registered with the DataProgressObserver.
  static void DataProgressCallbackFunction(vtkObject*, unsigned long, void*,
                                           void*);
  // Progress callback from XMLParser.
  virtual void DataProgressCallback();
  
  // The number of Pieces of data found in the file.
  int NumberOfPieces;
  
  // The PointData and CellData element representations for each piece.
  vtkVisItXMLDataElement** PointDataElements;
  vtkVisItXMLDataElement** CellDataElements;
  
  // The piece currently being read.
  int Piece;
  
  // The number of point/cell data arrays in the output.  Valid after
  // SetupOutputData has been called.
  int NumberOfPointArrays;
  int NumberOfCellArrays;  
  
  // Flag for whether DataProgressCallback should actually update
  // progress.
  int InReadData;
  
  // The observer to report progress from reading data from XMLParser.
  vtkCallbackCommand* DataProgressObserver;  

  // Specify the last time step read, usefull to know if we need to rearead data
  // //PointData
  int *PointDataTimeStep;
  unsigned long *PointDataOffset;
  int PointDataNeedToReadTimeStep(vtkVisItXMLDataElement *eNested);

  //CellData
  int *CellDataTimeStep;
  unsigned long *CellDataOffset;
  int CellDataNeedToReadTimeStep(vtkVisItXMLDataElement *eNested);
 
private:
  vtkVisItXMLDataReader(const vtkVisItXMLDataReader&);  // Not implemented.
  void operator=(const vtkVisItXMLDataReader&);  // Not implemented.
};

#endif
