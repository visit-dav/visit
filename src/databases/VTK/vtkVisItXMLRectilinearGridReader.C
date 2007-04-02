/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLRectilinearGridReader.cxx,v $
  Language:  C++
  Date:      $Date: 2003/05/05 13:45:23 $
  Version:   $Revision: 1.8 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItXMLRectilinearGridReader.h"

#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkRectilinearGrid.h"
#include "vtkVisItXMLDataElement.h"
#include "vtkVisItXMLDataParser.h"

vtkCxxRevisionMacro(vtkVisItXMLRectilinearGridReader, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkVisItXMLRectilinearGridReader);

//----------------------------------------------------------------------------
vtkVisItXMLRectilinearGridReader::vtkVisItXMLRectilinearGridReader()
{
  // Copied from vtkRectilinearGridReader constructor:
  this->SetOutput(vtkRectilinearGrid::New());
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  this->Outputs[0]->ReleaseData();
  this->Outputs[0]->Delete();
  this->CoordinateElements = 0;
}

//----------------------------------------------------------------------------
vtkVisItXMLRectilinearGridReader::~vtkVisItXMLRectilinearGridReader()
{
  if(this->NumberOfPieces) { this->DestroyPieces(); }
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::SetOutput(vtkRectilinearGrid *output)
{
  this->Superclass::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
vtkRectilinearGrid* vtkVisItXMLRectilinearGridReader::GetOutput()
{
  if(this->NumberOfOutputs < 1)
    {
    return 0;
    }
  return static_cast<vtkRectilinearGrid*>(this->Outputs[0]);
}

//----------------------------------------------------------------------------
vtkRectilinearGrid* vtkVisItXMLRectilinearGridReader::GetOutput(int idx)
{
  return static_cast<vtkRectilinearGrid*>(this->Superclass::GetOutput(idx));
}

//----------------------------------------------------------------------------
const char* vtkVisItXMLRectilinearGridReader::GetDataSetName()
{
  return "RectilinearGrid";
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::SetOutputExtent(int* extent)
{
  this->GetOutput()->SetExtent(extent);
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::SetupPieces(int numPieces)
{
  this->Superclass::SetupPieces(numPieces);
  this->CoordinateElements = new vtkVisItXMLDataElement*[numPieces];
  int i;
  for(i=0;i < numPieces; ++i)
    {
    this->CoordinateElements[i] = 0;
    }
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::DestroyPieces()
{
  delete [] this->CoordinateElements;
  this->CoordinateElements = 0;
  this->Superclass::DestroyPieces();
}

//----------------------------------------------------------------------------
int vtkVisItXMLRectilinearGridReader::ReadPiece(vtkVisItXMLDataElement* ePiece)
{
  if(!this->Superclass::ReadPiece(ePiece)) { return 0; }
  
  // Find the Coordinates element in the piece.
  int i;
  this->CoordinateElements[this->Piece] = 0;
  for(i=0; i < ePiece->GetNumberOfNestedElements(); ++i)
    {
    vtkVisItXMLDataElement* eNested = ePiece->GetNestedElement(i);
    if((strcmp(eNested->GetName(), "Coordinates") == 0)
       && (eNested->GetNumberOfNestedElements() == 3))
      {
      this->CoordinateElements[this->Piece] = eNested;
      }
    }
  
  // If there is any volume, we require a Coordinates element.
  int* piecePointDimensions = this->PiecePointDimensions + this->Piece*3;
  if(!this->CoordinateElements[this->Piece] &&
     (piecePointDimensions[0] > 0) &&
     (piecePointDimensions[1] > 0) &&
     (piecePointDimensions[2] > 0))
    {
    vtkErrorMacro("A piece is missing its Coordinates element.");
    return 0;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::SetupOutputInformation()
{
  this->Superclass::SetupOutputInformation();  
  vtkRectilinearGrid* output = this->GetOutput();
  
  // Use the configuration of the first piece since all are the same.
  if(!this->CoordinateElements[0])
    {
    // Empty volume.
    return;
    }
  vtkVisItXMLDataElement* xc = this->CoordinateElements[0]->GetNestedElement(0);
  vtkVisItXMLDataElement* yc = this->CoordinateElements[0]->GetNestedElement(1);
  vtkVisItXMLDataElement* zc = this->CoordinateElements[0]->GetNestedElement(2);
  
  // Create the coordinate arrays.
  vtkDataArray* x = this->CreateDataArray(xc);
  vtkDataArray* y = this->CreateDataArray(yc);
  vtkDataArray* z = this->CreateDataArray(zc);
  if(x && y && z)
    {
    x->SetNumberOfTuples(this->PointDimensions[0]);
    y->SetNumberOfTuples(this->PointDimensions[1]);
    z->SetNumberOfTuples(this->PointDimensions[2]);
    output->SetXCoordinates(x);
    output->SetYCoordinates(y);
    output->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();
    }
  else
    {
    if(x) { x->Delete(); }
    if(y) { y->Delete(); }
    if(z) { z->Delete(); }
    this->InformationError = 1;
    }
}

//----------------------------------------------------------------------------
void vtkVisItXMLRectilinearGridReader::SetupOutputData()
{
  this->Superclass::SetupOutputData();
}

//----------------------------------------------------------------------------
int vtkVisItXMLRectilinearGridReader::ReadPieceData()
{
  // The amount of data read by the superclass's ReadPieceData comes
  // from point/cell data (we read point specifications here).
  int dims[3] = {0,0,0};
  this->ComputePointDimensions(this->SubExtent, dims);  
  vtkIdType superclassPieceSize =
    (this->NumberOfPointArrays*dims[0]*dims[1]*dims[2]+
     this->NumberOfCellArrays*(dims[0]-1)*(dims[1]-1)*(dims[2]-1));
  
  // Total amount of data in this piece comes from point/cell data
  // arrays and the point specifications themselves.
  vtkIdType totalPieceSize =
    superclassPieceSize + dims[0] + dims[1] + dims[2];
  if(totalPieceSize == 0)
    {
    totalPieceSize = 1;
    }
  
  // Split the progress range based on the approximate fraction of
  // data that will be read by each step in this method.
  float progressRange[2] = {0,0};
  this->GetProgressRange(progressRange);
  float fractions[5] =
    {
      0,
      float(superclassPieceSize) / totalPieceSize,
      (float(superclassPieceSize)+dims[0]) / totalPieceSize,
      (float(superclassPieceSize)+dims[1]+dims[2]) / totalPieceSize,
      1
    };
  
  // Set the range of progress for the superclass.
  this->SetProgressRange(progressRange, 0, fractions);
  
  // Let the superclass read its data.
  if(!this->Superclass::ReadPieceData()) { return 0; }
  
  int index=this->Piece;
  vtkVisItXMLDataElement* xc = this->CoordinateElements[index]->GetNestedElement(0);
  vtkVisItXMLDataElement* yc = this->CoordinateElements[index]->GetNestedElement(1);
  vtkVisItXMLDataElement* zc = this->CoordinateElements[index]->GetNestedElement(2);
  int* pieceExtent = this->PieceExtents + index*6;
  vtkRectilinearGrid* output = this->GetOutput();
  int result = 1;
  
  // Set the range of progress for the X coordinates array.
  this->SetProgressRange(progressRange, 1, fractions);
  if(result)
    {
    this->ReadSubCoordinates(pieceExtent, this->UpdateExtent,
                             this->SubExtent, xc,
                             output->GetXCoordinates());
    }
  
  // Set the range of progress for the Y coordinates array.
  this->SetProgressRange(progressRange, 2, fractions);
  if(result)
    {
    this->ReadSubCoordinates(pieceExtent+2, this->UpdateExtent+2,
                             this->SubExtent+2, yc,
                             output->GetYCoordinates());
    }
  
  // Set the range of progress for the Z coordinates array.
  this->SetProgressRange(progressRange, 3, fractions);
  if(result)
    {
    this->ReadSubCoordinates(pieceExtent+4, this->UpdateExtent+4,
                             this->SubExtent+4, zc,
                             output->GetZCoordinates());
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkVisItXMLRectilinearGridReader::ReadSubCoordinates(int* inBounds,
                                                    int* outBounds,
                                                    int* subBounds,
                                                    vtkVisItXMLDataElement* da,
                                                    vtkDataArray* array)
{
  unsigned int components = array->GetNumberOfComponents();
  
  int destStartIndex = subBounds[0] - outBounds[0];
  int sourceStartIndex = subBounds[0] - inBounds[0];
  int length = subBounds[1] - subBounds[0] + 1;
  
  return this->ReadData(da, array->GetVoidPointer(destStartIndex*components),
                        array->GetDataType(), sourceStartIndex, length);
}
