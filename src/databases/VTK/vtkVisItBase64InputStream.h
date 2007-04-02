/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItBase64InputStream.h,v $
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
// .NAME vtkVisItBase64InputStream - Reads base64-encoded input from a stream.
// .SECTION Description
// vtkVisItBase64InputStream implements base64 decoding with the
// vtkVisItInputStream interface.

#ifndef __vtkVisItBase64InputStream_h
#define __vtkVisItBase64InputStream_h

#include "vtkVisItInputStream.h"

class vtkVisItBase64InputStream : public vtkVisItInputStream
{
public:
  vtkTypeRevisionMacro(vtkVisItBase64InputStream,vtkVisItInputStream);
  static vtkVisItBase64InputStream *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Called after the stream position has been set by the caller, but
  // before any Seek or Read calls.  The stream position should not be
  // adjusted by the caller until after an EndReading call.
  void StartReading();
  
  // Description:
  // Seek to the given offset in the input data.  Returns 1 for
  // success, 0 for failure.
  int Seek(unsigned long offset);
  
  // Description:
  // Read input data of the given length.  Returns amount actually
  // read.
  unsigned long Read(unsigned char* data, unsigned long length);
  
  // Description:
  // Called after all desired calls to Seek and Read have been made.
  // After this call, the caller is free to change the position of the
  // stream.  Additional reads should not be done until after another
  // call to StartReading.
  void EndReading();
protected:
  vtkVisItBase64InputStream();
  ~vtkVisItBase64InputStream();  
  
  // Number of decoded bytes left in Buffer from last call to Read.
  int BufferLength;
  unsigned char Buffer[2];
  
  // Reads 4 bytes from the input stream and decodes them into 3 bytes.
  int DecodeTriplet(unsigned char& c0, unsigned char& c1, unsigned char& c2);

private:
  vtkVisItBase64InputStream(const vtkVisItBase64InputStream&);  // Not implemented.
  void operator=(const vtkVisItBase64InputStream&);  // Not implemented.
};

#endif
