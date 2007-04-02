/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLImageDataReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLImageDataReader - Read VTK XML ImageData files.
// .SECTION Description
// vtkVisItXMLImageDataReader reads the VTK XML ImageData file format.  One
// image data file can be read to produce one output.  Streaming is
// supported.  The standard extension for this reader's file format is
// "vti".  This reader is also used to read a single piece of the
// parallel file format.

// .SECTION See Also
// vtkVisItXMLPImageDataReader

#ifndef __vtkVisItXMLImageDataReader_h
#define __vtkVisItXMLImageDataReader_h

#include "vtkVisItXMLStructuredDataReader.h"

class vtkImageData;

class vtkVisItXMLImageDataReader : public vtkVisItXMLStructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLImageDataReader,vtkVisItXMLStructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkVisItXMLImageDataReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkImageData *output);
  vtkImageData *GetOutput();
  vtkImageData *GetOutput(int idx);

  // Description:
  // For the specified port, copy the information this reader sets up in
  // SetupOutputInformation to outInfo
  virtual void CopyOutputInformation(vtkInformation *outInfo, int port);
  
protected:
  vtkVisItXMLImageDataReader();
  ~vtkVisItXMLImageDataReader();  
  
  double Origin[3];
  double Spacing[3];
  
  const char* GetDataSetName();
  void SetOutputExtent(int* extent);
  int ReadPrimaryElement(vtkVisItXMLDataElement* ePrimary);

  // Setup the output's information.
  void SetupOutputInformation(vtkInformation *outInfo);

  virtual int FillOutputPortInformation(int, vtkInformation*);
  
private:
  vtkVisItXMLImageDataReader(const vtkVisItXMLImageDataReader&);  // Not implemented.
  void operator=(const vtkVisItXMLImageDataReader&);  // Not implemented.
};

#endif
