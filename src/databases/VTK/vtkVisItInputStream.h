/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItInputStream.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItInputStream - Wraps a binary input stream with a VTK interface.
// .SECTION Description
// vtkVisItInputStream provides a VTK-style interface wrapping around a
// standard input stream.  The access methods are virtual so that
// subclasses can transparently provide decoding of an encoded stream.
// Data lengths for Seek and Read calls refer to the length of the
// input data.  The actual length in the stream may differ for
// subclasses that implement an encoding scheme.

#ifndef __vtkVisItInputStream_h
#define __vtkVisItInputStream_h

#include "vtkObject.h"

class vtkVisItInputStream : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkVisItInputStream,vtkObject);
  static vtkVisItInputStream *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //BTX
  // Description:
  // Get/Set the real input stream.
  vtkSetMacro(Stream, istream*);
  vtkGetMacro(Stream, istream*);
  //ETX
  
  // Description:  
  // Called after the stream position has been set by the caller, but
  // before any Seek or Read calls.  The stream position should not be
  // adjusted by the caller until after an EndReading call.
  virtual void StartReading();
  
  // Description:
  // Seek to the given offset in the input data.  Returns 1 for
  // success, 0 for failure.
  virtual int Seek(unsigned long offset);
  
  // Description:
  // Read input data of the given length.  Returns amount actually
  // read.
  virtual unsigned long Read(unsigned char* data, unsigned long length);
  unsigned long Read(char* data, unsigned long length);
  
  // Description:
  // Called after all desired calls to Seek and Read have been made.
  // After this call, the caller is free to change the position of the
  // stream.  Additional reads should not be done until after another
  // call to StartReading.
  virtual void EndReading();
  
protected:
  vtkVisItInputStream();
  ~vtkVisItInputStream();  
  
  // The real input stream.
  istream* Stream;
  
  // The input stream's position when StartReading was called.
  unsigned long StreamStartPosition;
  
private:
  vtkVisItInputStream(const vtkVisItInputStream&);  // Not implemented.
  void operator=(const vtkVisItInputStream&);  // Not implemented.
};

#endif
