/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItBase64OutputStream.h,v $
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
// .NAME vtkVisItBase64OutputStream - Writes base64-encoded output to a stream.
// .SECTION Description
// vtkVisItBase64OutputStream implements base64 encoding with the
// vtkVisItOutputStream interface.

#ifndef __vtkVisItBase64OutputStream_h
#define __vtkVisItBase64OutputStream_h

#include "vtkVisItOutputStream.h"

class vtkVisItBase64OutputStream : public vtkVisItOutputStream
{
public:
  vtkTypeRevisionMacro(vtkVisItBase64OutputStream,vtkVisItOutputStream);
  static vtkVisItBase64OutputStream *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:  
  // Called after the stream position has been set by the caller, but
  // before any Write calls.  The stream position should not be
  // adjusted by the caller until after an EndWriting call.
  int StartWriting();
  
  // Description:
  // Write output data of the given length.
  int Write(const unsigned char* data, unsigned long length);
  
  // Description:
  // Called after all desired calls to Write have been made.  After
  // this call, the caller is free to change the position of the
  // stream.  Additional writes should not be done until after another
  // call to StartWriting.
  int EndWriting();
  
protected:
  vtkVisItBase64OutputStream();
  ~vtkVisItBase64OutputStream();  
  
  // Number of un-encoded bytes left in Buffer from last call to Write.
  unsigned int BufferLength;
  unsigned char Buffer[2];
  
  // Methods to encode and write data.
  int EncodeTriplet(unsigned char c0, unsigned char c1, unsigned char c2);
  int EncodeEnding(unsigned char c0, unsigned char c1);
  int EncodeEnding(unsigned char c0);
  
private:
  vtkVisItBase64OutputStream(const vtkVisItBase64OutputStream&);  // Not implemented.
  void operator=(const vtkVisItBase64OutputStream&);  // Not implemented.
};

#endif
