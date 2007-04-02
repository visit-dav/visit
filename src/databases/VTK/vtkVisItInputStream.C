/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItInputStream.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItInputStream.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkVisItInputStream, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkVisItInputStream);

//----------------------------------------------------------------------------
vtkVisItInputStream::vtkVisItInputStream()
{
  this->Stream = 0;
}

//----------------------------------------------------------------------------
vtkVisItInputStream::~vtkVisItInputStream()
{
  this->SetStream(0);
}

//----------------------------------------------------------------------------
void vtkVisItInputStream::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Stream: " << (this->Stream? "set" : "none") << "\n";
}

//----------------------------------------------------------------------------
void vtkVisItInputStream::StartReading()
{
  if(!this->Stream)
    {
    vtkErrorMacro("StartReading() called with no Stream set.");
    }
  this->StreamStartPosition = this->Stream->tellg();
}

//----------------------------------------------------------------------------
void vtkVisItInputStream::EndReading()
{
}

//----------------------------------------------------------------------------
int vtkVisItInputStream::Seek(unsigned long offset)
{
  return (this->Stream->seekg(this->StreamStartPosition+offset)? 1:0);
}

//----------------------------------------------------------------------------
unsigned long vtkVisItInputStream::Read(unsigned char* data, unsigned long length)
{
  return this->Read(reinterpret_cast<char*>(data), length);
}

//----------------------------------------------------------------------------
unsigned long vtkVisItInputStream::Read(char* data, unsigned long length)
{
  this->Stream->read(data, length);
  return this->Stream->gcount();
}
