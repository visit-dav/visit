/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOutputStream.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItOutputStream.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkVisItOutputStream, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkVisItOutputStream);

//----------------------------------------------------------------------------
vtkVisItOutputStream::vtkVisItOutputStream()
{
  this->Stream = 0;
}

//----------------------------------------------------------------------------
vtkVisItOutputStream::~vtkVisItOutputStream()
{
  this->SetStream(0);
}

//----------------------------------------------------------------------------
void vtkVisItOutputStream::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Stream: " << (this->Stream? "set" : "none") << "\n";
}

//----------------------------------------------------------------------------
int vtkVisItOutputStream::StartWriting()
{
  if(!this->Stream)
    {
    vtkErrorMacro("StartWriting() called with no Stream set.");
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkVisItOutputStream::EndWriting()
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkVisItOutputStream::Write(const unsigned char* data, unsigned long length)
{
  return this->Write(reinterpret_cast<const char*>(data), length);
}

//----------------------------------------------------------------------------
int vtkVisItOutputStream::Write(const char* data, unsigned long length)
{
  return (this->Stream->write(data, length)? 1:0);
}
