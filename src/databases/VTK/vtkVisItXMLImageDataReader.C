/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLImageDataReader.cxx,v $
  Language:  C++
  Date:      $Date: 2003/02/07 20:06:29 $
  Version:   $Revision: 1.3 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
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

vtkCxxRevisionMacro(vtkVisItXMLImageDataReader, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkVisItXMLImageDataReader);

//----------------------------------------------------------------------------
vtkVisItXMLImageDataReader::vtkVisItXMLImageDataReader()
{
  // Copied from vtkImageDataReader constructor:
  this->SetOutput(vtkImageData::New());
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  this->Outputs[0]->ReleaseData();
  this->Outputs[0]->Delete();
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
  this->Superclass::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
vtkImageData* vtkVisItXMLImageDataReader::GetOutput()
{
  if(this->NumberOfOutputs < 1)
    {
    return 0;
    }
  return static_cast<vtkImageData*>(this->Outputs[0]);
}

//----------------------------------------------------------------------------
vtkImageData* vtkVisItXMLImageDataReader::GetOutput(int idx)
{
  return static_cast<vtkImageData*>(this->Superclass::GetOutput(idx));
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
void vtkVisItXMLImageDataReader::SetupOutputInformation()
{
  this->Superclass::SetupOutputInformation();
  
  // Backward-compatability support for scalar information in output.
  vtkImageData* output = this->GetOutput();
  
  output->SetOrigin(this->Origin);
  output->SetSpacing(this->Spacing);
  
  vtkDataArray* scalars = output->GetPointData()->GetScalars();
  if(scalars)
    {
    output->SetScalarType(scalars->GetDataType());
    output->SetNumberOfScalarComponents(scalars->GetNumberOfComponents());
    }  
}
