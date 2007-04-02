/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLImageDataReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItXMLImageDataReader.h"

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkVisItXMLDataElement.h"
#include "vtkInformation.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkCxxRevisionMacro(vtkVisItXMLImageDataReader, "$Revision: 1.11 $");
vtkStandardNewMacro(vtkVisItXMLImageDataReader);

//----------------------------------------------------------------------------
vtkVisItXMLImageDataReader::vtkVisItXMLImageDataReader()
{
  vtkImageData *output = vtkImageData::New();
  this->SetOutput(output);
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  output->ReleaseData();
  output->Delete();
}

//----------------------------------------------------------------------------
vtkVisItXMLImageDataReader::~vtkVisItXMLImageDataReader()
{
}

//----------------------------------------------------------------------------
void vtkVisItXMLImageDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkVisItXMLImageDataReader::SetOutput(vtkImageData *output)
{
  this->GetExecutive()->SetOutputData(0, output);
}

//----------------------------------------------------------------------------
vtkImageData* vtkVisItXMLImageDataReader::GetOutput()
{
  return this->GetOutput(0);
}

//----------------------------------------------------------------------------
vtkImageData* vtkVisItXMLImageDataReader::GetOutput(int idx)
{
  return vtkImageData::SafeDownCast( this->GetOutputDataObject(idx) );
}


//----------------------------------------------------------------------------
const char* vtkVisItXMLImageDataReader::GetDataSetName()
{
  return "ImageData";
}

//----------------------------------------------------------------------------
void vtkVisItXMLImageDataReader::SetOutputExtent(int* extent)
{
  this->GetOutput()->SetExtent(extent);
}

//----------------------------------------------------------------------------
int vtkVisItXMLImageDataReader::ReadPrimaryElement(vtkVisItXMLDataElement* ePrimary)
{
  if(!this->Superclass::ReadPrimaryElement(ePrimary)) { return 0; }
  
  // Get the image's origin.
  if(ePrimary->GetVectorAttribute("Origin", 3, this->Origin) != 3)
    {
    this->Origin[0] = 0;
    this->Origin[1] = 0;
    this->Origin[2] = 0;
    }
  
  // Get the image's spacing.
  if(ePrimary->GetVectorAttribute("Spacing", 3, this->Spacing) != 3)
    {
    this->Spacing[0] = 1;
    this->Spacing[1] = 1;
    this->Spacing[2] = 1;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
// Note that any changes (add or removing information) made to this method
// should be replicated in CopyOutputInformation
void vtkVisItXMLImageDataReader::SetupOutputInformation(vtkInformation *outInfo)
{
  this->Superclass::SetupOutputInformation(outInfo);

  outInfo->Set(vtkDataObject::ORIGIN(), this->Origin, 3);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);
}


//----------------------------------------------------------------------------
void vtkVisItXMLImageDataReader::CopyOutputInformation(vtkInformation *outInfo, int port)
{
  this->Superclass::CopyOutputInformation(outInfo, port);
  vtkInformation *localInfo = this->GetExecutive()->GetOutputInformation( port );
  
  if ( localInfo->Has(vtkDataObject::ORIGIN()) )
    {
    outInfo->CopyEntry( localInfo, vtkDataObject::ORIGIN() );
    }
  if ( localInfo->Has(vtkDataObject::SPACING()) )
    {
    outInfo->CopyEntry( localInfo, vtkDataObject::SPACING() );
    }
}


//----------------------------------------------------------------------------
int vtkVisItXMLImageDataReader::FillOutputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
  return 1;
}

