/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVisItXMLPDataReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLPDataReader - Superclass for PVTK XML file readers.
// .SECTION Description
// vtkXMLPDataReader provides functionality common to all PVTK XML
// file readers.  Concrete subclasses call upon this functionality
// when needed.

// .SECTION See Also
// vtkXMLDataReader

#ifndef __vtkVisItXMLPDataReader_h
#define __vtkVisItXMLPDataReader_h

#include "vtkXMLReader.h"

class vtkDataArray;
class vtkDataSet;
class vtkXMLDataReader;

class vtkVisItXMLPDataReader : public vtkXMLReader
{
public:
  vtkTypeMacro(vtkVisItXMLPDataReader,vtkXMLReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkVisItXMLPDataReader *New();
  
  // Description:
  // Get the number of pieces from the summary file being read.
  vtkGetMacro(NumberOfPieces, int);  
  
  // Description:
  // Get the ghost level.
  vtkGetMacro(GhostLevel, int);

  // Description:
  // Get the number of point data arrays.
  vtkGetMacro(NumberOfPointArrays, int);

  // Description:
  // Get the number of cell data arrays.
  vtkGetMacro(NumberOfCellArrays, int);

  // Description:
  // Get the name of the specified point data array.
  char* GetPointArrayName(int);

  // Description:
  // Get the name of the specified cell data array.
  char* GetCellArrayName(int);

  // Description:
  // Get the name of the specified piece file.
  char* GetPieceFileName(int);

  // For the specified port, copy the information this reader sets up in
  // SetupOutputInformation to outInfo
  virtual void CopyOutputInformation(vtkInformation *outInfo, int port);

  int ReadXMLInformation();

protected:
  vtkVisItXMLPDataReader();
  ~vtkVisItXMLPDataReader();
  
  void SetupEmptyOutput();

  const char* GetDataSetName();

  // Pipeline execute information driver.  Called by vtkXMLReader.
  virtual void SetupOutputInformation(vtkInformation *outInfo);

  int ReadPrimaryElement(vtkXMLDataElement* ePrimary);
  
  void SetupOutputData();
  
  virtual void SetupPieces(int numPieces);
  virtual void DestroyPieces();
  int ReadPiece(vtkXMLDataElement* ePiece, int index);
  
  char* CreatePieceFileName(const char* fileName);
  void SplitFileName();
  
  // Callback registered with the PieceProgressObserver.
  static void PieceProgressCallbackFunction(vtkObject*, unsigned long, void*,
                                           void*);
  virtual void PieceProgressCallback();
  
  // Pieces from the input summary file.
  int NumberOfPieces;
  
  // The ghost level available on each input piece.
  int GhostLevel;
  
  // The piece currently being read.
  int Piece;
  
  // The path to the input file without the file name.
  char* PathName;
  
  // The extension of the file name.
  char* Extension;

  // The number of point arrays.
  int NumberOfPointArrays;

  // The number of cell arrays.
  int NumberOfCellArrays;

  // The names of the piece files.
  char** PieceFileNames;

  // Information per-piece.
  vtkXMLDataElement** PieceElements;
  
  // The PPointData and PCellData element representations.
  vtkXMLDataElement* PPointDataElement;
  vtkXMLDataElement* PCellDataElement;  
  
  // The observer to report progress from reading serial data in each
  // piece.
  vtkCallbackCommand* PieceProgressObserver;  
  
private:
  vtkVisItXMLPDataReader(const vtkVisItXMLPDataReader&);  // Not implemented.
  void operator=(const vtkVisItXMLPDataReader&);  // Not implemented.
};

#endif
