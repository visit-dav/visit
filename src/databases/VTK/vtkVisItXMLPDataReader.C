/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVisItXMLPDataReader.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItXMLPDataReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDataArraySelection.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataReader.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkVisItXMLPDataReader);

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::SetupEmptyOutput()
{
  this->GetCurrentOutput()->Initialize();
}

//----------------------------------------------------------------------------
const char* vtkVisItXMLPDataReader::GetDataSetName()
{
  // This must match the type tag in the file or the file won't be
  // read properly.
  if (strncmp(this->Extension, "pvtu", 4) == 0)
    return "PUnstructuredGrid";
  else if (strncmp(this->Extension, "pvts", 4) == 0)
    return "PStructuredGrid";
  else if (strncmp(this->Extension, "pvtr", 4) == 0)
    return "PRectilinearGrid";
  else if (strncmp(this->Extension, "pvti", 4) == 0)
    return "PImage";
  else if (strncmp(this->Extension, "pvtp", 4) == 0)
    return "PPolyData";
  else
    return "None";
}

//----------------------------------------------------------------------------
vtkVisItXMLPDataReader::vtkVisItXMLPDataReader()
{
  this->GhostLevel = 0;
  
  this->NumberOfPieces = 0;
  
  this->PieceElements = 0;
  
  this->PathName = 0;

  this->Extension = 0;
  
  // Setup a callback for the internal serial readers to report
  // progress.
  this->PieceProgressObserver = vtkCallbackCommand::New();
  this->PieceProgressObserver->SetCallback(&vtkVisItXMLPDataReader::PieceProgressCallbackFunction);
  this->PieceProgressObserver->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkVisItXMLPDataReader::~vtkVisItXMLPDataReader()
{
  if(this->NumberOfPieces)
    {
    this->DestroyPieces();
    }
  if(this->PathName)
    {
    delete [] this->PathName;
    }
  this->PieceProgressObserver->Delete();
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfPieces: " << this->NumberOfPieces << "\n";
}


//----------------------------------------------------------------------------
char* vtkVisItXMLPDataReader::GetPointArrayName(int piece)
{
  return NULL;
}

//----------------------------------------------------------------------------
char* vtkVisItXMLPDataReader::GetCellArrayName(int piece)
{
  return NULL;
}

//----------------------------------------------------------------------------
char* vtkVisItXMLPDataReader::GetPieceFileName(int piece)
{
  return this->PieceFileNames[piece];
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::SetupOutputData()
{
  this->Superclass::SetupOutputData();
}

//----------------------------------------------------------------------------
int vtkVisItXMLPDataReader::ReadXMLInformation()
{
  // First setup the filename components.
  this->SplitFileName();
  
  // Now proceed with reading the information.
  return this->Superclass::ReadXMLInformation();
}

//----------------------------------------------------------------------------
// Note that any changes (add or removing information) made to this method
// should be replicated in CopyOutputInformation
void vtkVisItXMLPDataReader::SetupOutputInformation(vtkInformation *outInfo)
{
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::CopyOutputInformation(vtkInformation *outInfo,
                                              int port)
{
#if 0
  vtkInformation *localInfo = 
    this->GetExecutive()->GetOutputInformation( port );
  if ( localInfo->Has(vtkDataObject::POINT_DATA_VECTOR()) )
    {
    outInfo->CopyEntry( localInfo, vtkDataObject::POINT_DATA_VECTOR() );
    }
  if ( localInfo->Has(vtkDataObject::CELL_DATA_VECTOR()) )
    {
    outInfo->CopyEntry( localInfo, vtkDataObject::CELL_DATA_VECTOR() );
    }
#endif
}


//----------------------------------------------------------------------------
int vtkVisItXMLPDataReader::ReadPrimaryElement(vtkXMLDataElement* ePrimary)
{
  if(!this->Superclass::ReadPrimaryElement(ePrimary))
    {
    return 0;
    }
  // Read information about the data.
  if(!ePrimary->GetScalarAttribute("GhostLevel", this->GhostLevel))
    {
    this->GhostLevel = 0;
    }
  
  // Read information about the pieces.
  this->PPointDataElement = 0;
  this->PCellDataElement = 0;
  int i;
  int numNested = ePrimary->GetNumberOfNestedElements();
  int numPieces = 0;
  for(i=0;i < numNested; ++i)
    {
    vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
    if(strcmp(eNested->GetName(), "Piece") == 0)
      {
      ++numPieces;
      }
    else if(strcmp(eNested->GetName(), "PPointData") == 0)
      {
      this->PPointDataElement = eNested;
      }
    else if(strcmp(eNested->GetName(), "PCellData") == 0)
      {
      this->PCellDataElement = eNested;
      }
    }
  this->SetupPieces(numPieces);
  int piece = 0;
  for(i=0;i < numNested; ++i)
    {
    vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
    if(strcmp(eNested->GetName(), "Piece") == 0)
      {
      if(!this->ReadPiece(eNested, piece++))
        {
        return 0;
        }
      }
    }
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::SetupPieces(int numPieces)
{
  if(this->NumberOfPieces)
    {
    this->DestroyPieces();
    }
  this->NumberOfPieces = numPieces;
  this->PieceFileNames = new char*[this->NumberOfPieces];
  int i;
  for(i=0;i < this->NumberOfPieces;++i)
    {
    this->PieceFileNames[i] = 0;
    }
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::DestroyPieces()
{
  int i;
  for(i=0;i < this->NumberOfPieces;++i)
    {
    if(this->PieceFileNames[i])
      delete [] this->PieceFileNames[i];
    }
  delete [] this->PieceFileNames;
  this->PieceFileNames = 0;
}

//----------------------------------------------------------------------------
int vtkVisItXMLPDataReader::ReadPiece(vtkXMLDataElement* ePiece, int index)
{
  const char* fileName = ePiece->GetAttribute("Source");
  if(!fileName)
    {
    vtkErrorMacro("Piece " << index << " has no Source attribute.");
    return 0;
    }

  // The file name is relative to the summary file.  Convert it to
  // something we can use.
  this->PieceFileNames[index] = this->CreatePieceFileName(fileName);
  
  return 1;
}

//----------------------------------------------------------------------------
char* vtkVisItXMLPDataReader::CreatePieceFileName(const char* fileName)
{
  vtksys_ios::ostringstream fn_with_warning_C4701;
  if(this->PathName)
    {
    fn_with_warning_C4701 << this->PathName;
    }
  fn_with_warning_C4701 << fileName;
  
  size_t len = fn_with_warning_C4701.str().length();
  char *buffer = new char[len + 1];
  strncpy(buffer, fn_with_warning_C4701.str().c_str(), len);
  buffer[len] = '\0';
  
  return buffer;
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::SplitFileName()
{
  if(!this->FileName)
    {
    vtkErrorMacro( << "Need to specify a filename" );
    return;
    }

  // Pull the PathName component out of the FileName.
  size_t length = strlen(this->FileName);
  char* fileName = new char[length+1];
  strcpy(fileName, this->FileName);
  char* begin = fileName;
  char* end = fileName + length;
  char* s;
  
#if defined(_WIN32)
  // Convert to UNIX-style slashes.
  for(s=begin;s != end;++s) { if(*s == '\\') { *s = '/'; } }
#endif
  
  // Extract the path name up to the last '/'.
  if(this->PathName) 
    { 
    delete [] this->PathName; 
    this->PathName = 0; 
    }
  char* rbegin = end-1;
  char* rend = begin-1;
  for(s=rbegin;s != rend;--s) 
    { 
    if(*s == '/') 
      { 
      break; 
      } 
    }
  if(s >= begin)
    {
    length = (s-begin)+1;
    this->PathName = new char[length+1];
    strncpy(this->PathName, this->FileName, length);
    this->PathName[length] = '\0';
    }
  
  // Pull out the extension (assume last 4 characters of filename).
  if(this->Extension) 
    { 
    delete [] this->Extension; 
    this->Extension = 0; 
    }
  length = 4;
  this->Extension = new char[length+1];
  strncpy(this->Extension, end-4, length);
  this->Extension[length] = '\0';

  // Cleanup temporary name.
  delete [] fileName;
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::PieceProgressCallbackFunction(vtkObject*,
  unsigned long, void* clientdata, void*)
{
  reinterpret_cast<vtkVisItXMLPDataReader*>(clientdata)->PieceProgressCallback();
}

//----------------------------------------------------------------------------
void vtkVisItXMLPDataReader::PieceProgressCallback()
{
  this->UpdateProgressDiscrete(this->ProgressRange[1]);
}
