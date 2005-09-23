/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItOutputStream.h,v $
  Language:  C++
  Date:      $Date: 2002/10/16 18:23:06 $
  Version:   $Revision: 1.1 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItOutputStream - Wraps a binary output stream with a VTK interface.
// .SECTION Description
// vtkVisItOutputStream provides a VTK-style interface wrapping around a
// standard output stream.  The access methods are virtual so that
// subclasses can transparently provide encoding of the output.  Data
// lengths for Write calls refer to the length of the data in memory.
// The actual length in the stream may differ for subclasses that
// implement an encoding scheme.

#ifndef __vtkVisItOutputStream_h
#define __vtkVisItOutputStream_h

#include "vtkObject.h"

class vtkVisItOutputStream : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkVisItOutputStream,vtkObject);
  static vtkVisItOutputStream *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //BTX
  // Description:
  // Get/Set the real output stream.
  vtkSetMacro(Stream, ostream*);
  vtkGetMacro(Stream, ostream*);
  //ETX
  
  // Description:  
  // Called after the stream position has been set by the caller, but
  // before any Write calls.  The stream position should not be
  // adjusted by the caller until after an EndWriting call.
  virtual int StartWriting();
  
  // Description:
  // Write output data of the given length.
  virtual int Write(const unsigned char* data, unsigned long length);
  int Write(const char* data, unsigned long length);
  
  // Description:
  // Called after all desired calls to Write have been made.  After
  // this call, the caller is free to change the position of the
  // stream.  Additional writes should not be done until after another
  // call to StartWriting.
  virtual int EndWriting();
  
protected:
  vtkVisItOutputStream();
  ~vtkVisItOutputStream();  
  
  // The real output stream.
  ostream* Stream;
  
private:
  vtkVisItOutputStream(const vtkVisItOutputStream&);  // Not implemented.
  void operator=(const vtkVisItOutputStream&);  // Not implemented.
};

#endif
