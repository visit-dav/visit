/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLDataReader.cxx,v $
  Language:  C++
  Date:      $Date: 2003/06/26 17:36:51 $
  Version:   $Revision: 1.7 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItXMLDataReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCellData.h"
#include "vtkDataArraySelection.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"
#include "vtkVisItXMLDataElement.h"
#include "vtkVisItXMLDataParser.h"

vtkCxxRevisionMacro(vtkVisItXMLDataReader, "$Revision: 1.7 $");

//----------------------------------------------------------------------------
vtkVisItXMLDataReader::vtkVisItXMLDataReader()
{
  this->NumberOfPieces = 0;
  this->PointDataElements = 0;
  this->CellDataElements = 0;
  this->Piece = 0;
  this->NumberOfPointArrays = 0;
  this->NumberOfCellArrays = 0;
  this->InReadData = 0;
  
  // Setup a callback for when the XMLParser's data reading routines
  // report progress.
  this->DataProgressObserver = vtkCallbackCommand::New();
  this->DataProgressObserver->SetCallback(&vtkVisItXMLDataReader::DataProgressCallbackFunction);
  this->DataProgressObserver->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkVisItXMLDataReader::~vtkVisItXMLDataReader()
{
  if(this->XMLParser) { this->DestroyXMLParser(); }
  if(this->NumberOfPieces) { this->DestroyPieces(); }
  this->DataProgressObserver->Delete();
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::CreateXMLParser()
{
  this->Superclass::CreateXMLParser();
  this->XMLParser->AddObserver(vtkCommand::ProgressEvent,
                               this->DataProgressObserver);
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::DestroyXMLParser()
{
  if(this->XMLParser)
    {
    this->XMLParser->RemoveObserver(this->DataProgressObserver);
    }
  this->Superclass::DestroyXMLParser();
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadPrimaryElement(vtkVisItXMLDataElement* ePrimary)
{
  if(!this->Superclass::ReadPrimaryElement(ePrimary)) { return 0; }
  
  // Count the number of pieces in the file.
  int numNested = ePrimary->GetNumberOfNestedElements();
  int numPieces = 0;
  int i;
  for(i=0; i < numNested; ++i)
    {
    vtkVisItXMLDataElement* eNested = ePrimary->GetNestedElement(i);
    if(strcmp(eNested->GetName(), "Piece") == 0) { ++numPieces; }
    }
  
  // Now read each piece.  If no "Piece" elements were found, assume
  // the primary element itself is a single piece.
  if(numPieces)
    {
    this->SetupPieces(numPieces);
    int piece = 0;
    for(i=0;i < numNested; ++i)
      {
      vtkVisItXMLDataElement* eNested = ePrimary->GetNestedElement(i);
      if(strcmp(eNested->GetName(), "Piece") == 0)
        {
        if(!this->ReadPiece(eNested, piece++)) { return 0; }
        }
      }
    }
  else
    {
    this->SetupPieces(1);
    if(!this->ReadPiece(ePrimary, 0)) { return 0; }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::SetupPieces(int numPieces)
{
  if(this->NumberOfPieces) { this->DestroyPieces(); }
  this->NumberOfPieces = numPieces;
  if (numPieces > 0)
    {
    this->PointDataElements = new vtkVisItXMLDataElement*[numPieces];
    this->CellDataElements = new vtkVisItXMLDataElement*[numPieces];
    }
  int i;
  for(i=0;i < this->NumberOfPieces;++i)
    {
    this->PointDataElements[i] = 0;
    this->CellDataElements[i] = 0;
    }
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::DestroyPieces()
{
  delete [] this->PointDataElements;
  delete [] this->CellDataElements;
  this->PointDataElements = 0;
  this->CellDataElements = 0;
  this->NumberOfPieces = 0;
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::SetupOutputInformation()
{
  this->Superclass::SetupOutputInformation();
  if(!this->NumberOfPieces) { return; }
  
  int i;
  vtkPointData* pointData = this->GetOutputAsDataSet(0)->GetPointData();
  vtkCellData* cellData = this->GetOutputAsDataSet(0)->GetCellData();  
 
  // Get the size of the output arrays.
  int pointTuples = this->GetNumberOfPoints();
  int cellTuples = this->GetNumberOfCells();

  // Allocate the arrays in the output.  We only need the information
  // from one piece because all pieces have the same set of arrays.
  vtkVisItXMLDataElement* ePointData = this->PointDataElements[0];
  vtkVisItXMLDataElement* eCellData = this->CellDataElements[0];

  // Setup the point and cell data arrays without allocation.
  this->NumberOfPointArrays = 0;
  this->SetDataArraySelections(ePointData, this->PointDataArraySelection); 
  if(ePointData)
    {
    for(i=0;i < ePointData->GetNumberOfNestedElements();++i)
      {
      vtkVisItXMLDataElement* eNested = ePointData->GetNestedElement(i);
      if(this->PointDataArrayIsEnabled(eNested))
        {
        this->NumberOfPointArrays++;
        vtkDataArray* array = this->CreateDataArray(eNested);
        if(array)
          {      
          array->SetNumberOfTuples(pointTuples);
          pointData->AddArray(array);
          array->Delete();
          }
        else
          {
          this->InformationError = 1;
          }
        }
      }
    }
  this->NumberOfCellArrays = 0;
  this->SetDataArraySelections(eCellData, this->CellDataArraySelection); 
  if(eCellData)
    {
    for(i=0;i < eCellData->GetNumberOfNestedElements();++i)
      {
      vtkVisItXMLDataElement* eNested = eCellData->GetNestedElement(i);
      if(this->CellDataArrayIsEnabled(eNested)) 
        {
        this->NumberOfCellArrays++;
        vtkDataArray* array = this->CreateDataArray(eNested);
        if(array)
          {
          array->SetNumberOfTuples(cellTuples);
          cellData->AddArray(array);
          array->Delete();
          }
        else
          {
          this->InformationError = 1;
          }
        }
      }
    }
  
  // Setup attribute indices for the point data and cell data.
  this->ReadAttributeIndices(ePointData, pointData);
  this->ReadAttributeIndices(eCellData, cellData);
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::SetupOutputData()
{
  this->Superclass::SetupOutputData();
  
  vtkDataSet* output = this->GetOutputAsDataSet(0);
  vtkPointData* pointData = output->GetPointData();
  vtkCellData* cellData = output->GetCellData();
  
  // Get the size of the output arrays.
  unsigned long pointTuples = this->GetNumberOfPoints();
  unsigned long cellTuples = this->GetNumberOfCells();
  
  // Allocate the arrays in the output.  We only need the information
  // from one piece because all pieces have the same set of arrays.
  vtkVisItXMLDataElement* ePointData = this->PointDataElements[0];
  vtkVisItXMLDataElement* eCellData = this->CellDataElements[0];
  int i;
  if(ePointData)
    {
    int a =0;
    for(i=0;i < ePointData->GetNumberOfNestedElements();++i)
      {
      vtkVisItXMLDataElement* eNested = ePointData->GetNestedElement(i);
      if(this->PointDataArrayIsEnabled(eNested) )
        {
        pointData->GetArray(a++)->SetNumberOfTuples(pointTuples);
        }
      }
    }
  if(eCellData)
    {
    int a = 0;
    for(i=0;i < eCellData->GetNumberOfNestedElements();++i)
      {
      vtkVisItXMLDataElement* eNested = eCellData->GetNestedElement(i);
      if(this->CellDataArrayIsEnabled(eNested)) 
        {
        cellData->GetArray(a++)->SetNumberOfTuples(cellTuples);
        }
      }
    }
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadPiece(vtkVisItXMLDataElement* ePiece, int piece)
{
  this->Piece = piece;
  return this->ReadPiece(ePiece);
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadPiece(vtkVisItXMLDataElement* ePiece)
{
  // Find the PointData and CellData in the piece.
  int i;
  for(i=0; i < ePiece->GetNumberOfNestedElements(); ++i)
    {
    vtkVisItXMLDataElement* eNested = ePiece->GetNestedElement(i);
    if(strcmp(eNested->GetName(), "PointData") == 0)
      {
      this->PointDataElements[this->Piece] = eNested;
      }
    else if(strcmp(eNested->GetName(), "CellData") == 0)
      {
      this->CellDataElements[this->Piece] = eNested;
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadPieceData(int piece)
{
  this->Piece = piece;
  return this->ReadPieceData();
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadPieceData()
{
  vtkPointData* pointData = this->GetOutputAsDataSet(0)->GetPointData();
  vtkCellData* cellData = this->GetOutputAsDataSet(0)->GetCellData();
  vtkVisItXMLDataElement* ePointData = this->PointDataElements[this->Piece];
  vtkVisItXMLDataElement* eCellData = this->CellDataElements[this->Piece];
  
  // Split current progress range over number of arrays.  This assumes
  // that each array contributes approximately the same amount of data
  // within this piece.
  float progressRange[2] = {0,0};
  int currentArray=0;
  int numArrays = this->NumberOfPointArrays + this->NumberOfCellArrays;
  this->GetProgressRange(progressRange);
  
  // Read the data for this piece from each array.
  int i;
  if(ePointData)
    {
    int a=0;
    for(i=0;(i < ePointData->GetNumberOfNestedElements() &&
             !this->AbortExecute);++i)
      {
      vtkVisItXMLDataElement* eNested = ePointData->GetNestedElement(i);
      if(this->PointDataArrayIsEnabled(eNested))
        {
        if (strcmp(eNested->GetName(), "DataArray") != 0)
          {
          vtkErrorMacro("Invalid DataArray");
          this->InformationError = 1;
          return 0;
          }
        // Set the range of progress for this array.
        this->SetProgressRange(progressRange, currentArray++, numArrays);
        
        // Read the array.
        if(!this->ReadArrayForPoints(eNested, pointData->GetArray(a++)))
          {
          vtkErrorMacro("Cannot read point data array \""
                        << pointData->GetArray(a-1)->GetName() << "\" from "
                        << ePointData->GetName() << " in piece " << this->Piece
                        << ".  The data array in the element may be too short.");
          return 0;
          }
        }
      }
    }
  if(eCellData)
    {
    int a=0;
    for(i=0;(i < eCellData->GetNumberOfNestedElements() &&
             !this->AbortExecute);++i)
      {
      vtkVisItXMLDataElement* eNested = eCellData->GetNestedElement(i);
      if(this->CellDataArrayIsEnabled(eNested))
        {
        if (strcmp(eNested->GetName(), "DataArray") != 0)
          {
          this->InformationError = 1;
          vtkErrorMacro("Invalid DataARray");
          return 0;
          }
        // Set the range of progress for this array.
        this->SetProgressRange(progressRange, currentArray++, numArrays);
        
        // Read the array.
        if(!this->ReadArrayForCells(eNested, cellData->GetArray(a++)))
          {
          vtkErrorMacro("Cannot read cell data array \""
                        << cellData->GetArray(a-1)->GetName() << "\" from "
                        << ePointData->GetName() << " in piece " << this->Piece
                        << ".  The data array in the element may be too short.");
          return 0;
          }
        }
      }
    }
  
  if(this->AbortExecute)
    {
    return 0;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadArrayForPoints(vtkVisItXMLDataElement* da,
                                         vtkDataArray* outArray)
{
  vtkIdType components = outArray->GetNumberOfComponents();
  vtkIdType numberOfTuples = this->GetNumberOfPoints();
  return this->ReadData(da, outArray->GetVoidPointer(0),
                        outArray->GetDataType(),
                        0, numberOfTuples*components);
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadArrayForCells(vtkVisItXMLDataElement* da,
                                        vtkDataArray* outArray)
{
  vtkIdType components = outArray->GetNumberOfComponents();
  vtkIdType numberOfTuples = this->GetNumberOfCells();
  return this->ReadData(da, outArray->GetVoidPointer(0),
                        outArray->GetDataType(),
                        0, numberOfTuples*components);
}

//----------------------------------------------------------------------------
int vtkVisItXMLDataReader::ReadData(vtkVisItXMLDataElement* da, void* data, int wordType,
                               int startWord, int numWords)
{
  // Skip real read if aborting.
  if(this->AbortExecute)
    {
    return 0;
    }
  
  this->InReadData = 1;
  unsigned long num = numWords;
  int result = 0;
  if(da->GetAttribute("offset"))
    {
    int offset = 0;
    da->GetScalarAttribute("offset", offset);
    result = (this->XMLParser->ReadAppendedData(offset, data, startWord,
                                                numWords, wordType) == num);
    }
  else
    {
    int isAscii = 1;
    const char* format = da->GetAttribute("format");
    if(format && (strcmp(format, "binary") == 0)) { isAscii = 0; }
    result = (this->XMLParser->ReadInlineData(da, isAscii, data,
                                              startWord, numWords, wordType)
              == num);
    }
  this->InReadData = 0;
  return result;
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::DataProgressCallbackFunction(vtkObject*, unsigned long,
                                                    void* clientdata, void*)
{
  reinterpret_cast<vtkVisItXMLDataReader*>(clientdata)->DataProgressCallback();
}

//----------------------------------------------------------------------------
void vtkVisItXMLDataReader::DataProgressCallback()
{
  if(this->InReadData)
    {
    float width = this->ProgressRange[1]-this->ProgressRange[0];
    float dataProgress = this->XMLParser->GetProgress();
    float progress = this->ProgressRange[0] + dataProgress*width;
    this->UpdateProgressDiscrete(progress);
    if(this->AbortExecute)
      {
      this->XMLParser->SetAbort(1);
      }
    }
}




